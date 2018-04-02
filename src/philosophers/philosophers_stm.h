#ifndef PHILOSOPHERS_STM_H
#define PHILOSOPHERS_STM_H

#include <functional>
#include <any>
#include <stm.h>

#include "philosophers_types.h"

namespace philosophers
{

using namespace stm;

const std::function<STML<ForkPair>(TForkPair)>
    readForks =
        [](const TForkPair& forks)
{
    STML<Fork> lm = readTVar(forks.left);
    STML<Fork> rm = readTVar(forks.right);
    return both(lm, rm, mkForkPair);
};

const std::function<STML<bool>(TFork)>
takeFork =
    [](const TFork& tFork)
{
    STML<Fork> mf = readTVar(tFork);
    return bind<Fork, bool>(mf, [=](const Fork& fork)
    {
        if (fork.state == ForkState::Taken)
        {
            return pure(false);
        }
        else
        {
            STML<fp::Unit> m = writeTVar(tFork, Fork {fork.name, ForkState::Taken});
            return bind<fp::Unit, bool>(m, [](auto){ return pure(true);});
        }
    });
};

const std::function<STML<bool>(TForkPair)>
    takeForks =
        [](const TForkPair& forks)
{
    STML<bool> lm = takeFork(forks.left);
    STML<bool> rm = takeFork(forks.right);
    return both<bool, bool, bool>(lm, rm, [](bool l, bool r) { return l && r; });
};

const std::function<STML<fp::Unit>(TFork)>
    putFork =
        [](const TFork& tFork)
{
    std::function<Fork(Fork)> f = [](const Fork& fork)
    {
        return Fork {fork.name, ForkState::Free};
    };
    return modifyTVar(tFork, f);
};

const std::function<STML<fp::Unit>(TForkPair)>
    putForks =
        [](const TForkPair& forks)
{
    STML<fp::Unit> lm = putFork(forks.left);
    STML<fp::Unit> rm = putFork(forks.right);
    return bothVoided(lm, rm);
};

const std::function<STML<Activity>(Philosopher)>
    changeActivity = [](const Philosopher& philosopher)
{
    STML<Activity> mAct = readTVar(philosopher.activity);
    return bind<Activity, Activity>(mAct, [=](Activity activity)
    {
        STML<Activity> mAct2;
        switch (activity)
        {
            case Activity::Thinking:
            {
                STML<bool> mTaken = takeForks(philosopher.forks);
                auto mNext1 = unless(mTaken, mRetry);
                STML<fp::Unit> mNext2 = bind<bool, fp::Unit>(mTaken, [&](auto)
                {
                    return writeTVar(philosopher.activity, Activity::Eating);
                });
                mAct2 = sequence(mNext2, pure(Activity::Eating));
            }
            break;

            case Activity::Eating:
            {
                STML<fp::Unit> ma = putForks(philosopher.forks);
                STML<fp::Unit> mb = sequence(ma, writeTVar(philosopher.activity, Activity::Thinking));
                mAct2 = sequence(mb, pure(Activity::Thinking));
            }
            break;
        };
        return mAct2;
    });
};

const std::function<STML<int>(Philosopher)>
    incrementCycle = [](const Philosopher& philosopher)
{
    std::function<int(int)> f = [](int i) { return i + 1; };
    STML<fp::Unit> ma = modifyTVar(philosopher.cycle, f);
    return stm::bind<fp::Unit, int>(ma, [=](const auto&) { return readTVar(philosopher.cycle); });
};

} // namespace philosophers

#endif // PHILOSOPHERS_STM_H
