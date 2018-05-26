#ifndef PHILOSOPHERS_STM_H
#define PHILOSOPHERS_STM_H

#include <functional>
#include <any>
#include <stm.h>

#include "philosophers_types.h"

namespace philosophers
{

using namespace stm;

const std::function<Fork(Fork)>
setForkTaken = [](const Fork& fork)
{
    return Fork {fork.name, ForkState::Taken};
};

const std::function<Fork(Fork)>
setForkFree = [](const Fork& fork)
{
    return Fork {fork.name, ForkState::Free};
};

const std::function<int(int)>
increment = [](int i)
{
    return i + 1;
};

STML<Unit> takeFork(const TFork& tFork)
{
    // Warning! Passing closure by ref leads to crash.
    return withTVar<Fork, Unit>(tFork, [=](const Fork& fork)
    {
       if (fork.state == ForkState::Free)
       {
           return modifyTVar<Fork>(tFork, setForkTaken);
       }
       else
       {
           return retry<Unit>();
       }
    });
}

STML<Unit> takeForks(const TForkPair& forks)
{
    STML<Unit> lm = takeFork(forks.left);
    STML<Unit> rm = takeFork(forks.right);
    return sequence(lm, rm);
}

STML<Unit> putFork(const TFork& tFork)
{
    return modifyTVar<Fork>(tFork, setForkFree);
}

STML<Unit> putForks(const TForkPair& forks)
{
    STML<Unit> lm = putFork(forks.left);
    STML<Unit> rm = putFork(forks.right);
    return bothVoided(lm, rm);
}

STML<Activity> changeActivity(const Philosopher& philosopher)
{
    STML<Activity> mAct = readTVar(philosopher.activity);
    STML<Unit> changed = bind<Activity, Unit>(mAct, [=](Activity oldAct)
        {
            if (oldAct == Activity::Thinking)
            {
                STML<Unit> taken = takeForks(philosopher.forks);
                return sequence<Unit, Unit>(taken, writeTVar<Activity>(philosopher.activity, Activity::Eating));
            }
            else
            {
                STML<Unit> freed = putForks(philosopher.forks);
                return sequence<Unit, Unit>(freed, writeTVar<Activity>(philosopher.activity, Activity::Thinking));
            }
        });
    return sequence<Unit, Activity>(changed, readTVar<Activity>(philosopher.activity));
}

const std::function<STML<int>(Philosopher)>
incrementCycle = [](const Philosopher& philosopher)
{
    return modifyTVarRet<int>(philosopher.cycle, increment);
};

} // namespace philosophers

#endif // PHILOSOPHERS_STM_H
