#include <iostream>
#include <mutex>
#include <list>
#include <thread>
#include <chrono>

using namespace std;

#include <stm.h>

#include "philosophers_types.h"
#include "philosophers_stm.h"

using namespace philosophers;

using Philosophers = std::list<Philosopher>;

struct Shot
{
    std::string name;
    int cycles;
    Activity activity;
    Fork leftFork;
    Fork rightFork;
};

struct Snapshot
{
    std::list<Shot> shots;
    int number;
};

void printSnapshot(std::mutex& logLock, const Snapshot& snapshot)
{
    logLock.lock();

    std::cout << "\nSnapshot #" << snapshot.number << std::endl;
    for (const Shot& shot: snapshot.shots)
    {
        std::cout << "  [" << shot.name << "] (" << shot.cycles << ") "
                  << printActivity(shot.activity) << ", "
                  << printFork(shot.leftFork) << ":"
                  << printFork(shot.rightFork) << endl;
    }

    logLock.unlock();
}

Philosopher mkPhilosopher(Context& context,
                          const std::string& name,
                          const TFork& l,
                          const TFork& r)
{
    auto tActivity = stm::newTVarIO(context, Activity::Thinking);
    auto tCycles   = stm::newTVarIO(context, 0);
    return Philosopher { name, tCycles, tActivity, TForkPair {l, r}};
}

stm::STML<Shot> takeShot(const Philosopher& p)
{
    // TODO: readMany
    return stm::bind<Activity, Shot>(stm::readTVar(p.activity),             [=](Activity act)
    {
        return stm::bind<int, Shot>(stm::readTVar(p.cycles),                [=](int cycles)
        {
            return stm::bind<Fork, Shot>(stm::readTVar(p.forks.left),       [=](Fork l)
            {
                return stm::bind<Fork, Shot>(stm::readTVar(p.forks.right),  [=](Fork r)
                {
                    return stm::pure(Shot {
                                         p.name,
                                         cycles,
                                         act,
                                         l,
                                         r
                                     });
                });
            });
        });
    });
}

stm::STML<std::list<Shot>>
readPhilosophers(Philosophers& ps)
{
    if (ps.size() == 0)
    {
        return stm::pure(std::list<Shot>());
    }
    else
    {
        auto mShot = takeShot(ps.front());
        ps.pop_front();

        return stm::bind<Shot, std::list<Shot>>(mShot, [&](auto shot)
        {
            stm::STML<std::list<Shot>> mss;
            mss = readPhilosophers(ps);
            return stm::bind<std::list<Shot>, std::list<Shot>>(mss, [=](const std::list<Shot>& lst)
            {
                std::list<Shot> lst2 = std::move(lst);
                lst2.push_front(shot);
                return stm::pure(lst2);
            });
        });
    }
}

Snapshot takeSnapshot(Context& context, const Philosophers& ps, int number)
{
    Philosophers ps2 = ps;
    STML<std::list<Shot>> mss = readPhilosophers(ps2);

    auto snapshots = stm::atomically(context, mss);
    return Snapshot { snapshots, number };
}

struct Rt
{
    std::mutex& logLock;
    Context& context;
    Philosophers& philosophers;
};

void monitoring(Rt rt)
{
    std::cout << "Monitoring started." << endl;
    for (int i = 1; i < 10; ++i)
    {
        std::chrono::microseconds interval(1000 * 1000 * 5);
        std::this_thread::sleep_for(interval);

        auto snapshot = takeSnapshot(rt.context, rt.philosophers, i);
        printSnapshot(rt.logLock, snapshot);
    }
    std::cout << "Monitoring ended." << endl;
}

void runPhilosophers()
{
    stm::Context context;

    TFork tFork1 = stm::newTVarIO(context, Fork {"1", ForkState::Free });
    TFork tFork2 = stm::newTVarIO(context, Fork {"2", ForkState::Free });
    TFork tFork3 = stm::newTVarIO(context, Fork {"3", ForkState::Free });
    TFork tFork4 = stm::newTVarIO(context, Fork {"4", ForkState::Free });
    TFork tFork5 = stm::newTVarIO(context, Fork {"5", ForkState::Free });

    Philosophers philosophers =
        { mkPhilosopher(context, "1", tFork1, tFork2)
        , mkPhilosopher(context, "2", tFork2, tFork3)
        , mkPhilosopher(context, "3", tFork3, tFork4)
        , mkPhilosopher(context, "4", tFork4, tFork5)
        , mkPhilosopher(context, "5", tFork5, tFork1)
        };

    auto logLock = std::mutex();

    auto snapshot = takeSnapshot(context, philosophers, 0);
    printSnapshot(logLock, snapshot);
    std::thread t = std::thread(monitoring, Rt { logLock, context, philosophers });
    t.join();

//    std::chrono::microseconds interval(1000 * 1000 * 5 * 10);
//    std::this_thread::sleep_for(interval);
}

int main()
{
    runPhilosophers();
    return 0;
}
