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

const std::function<bool(Fork)>
isForkTaken = [](const Fork& fork)
{
    return fork.state == ForkState::Taken;
};

const std::function<Fork(Fork)>
setForkTaken = [](const Fork& fork)
{
    return Fork {fork.name, ForkState::Taken};
};

const std::function<int(int)>
increment = [](int i)
{
    return i + 1;
};

const std::function<Fork(Fork)>
setFree = [](const Fork& fork)
{
    return Fork {fork.name, ForkState::Free};
};

STML<bool> takeFork(const TFork& tFork)
{
    STML<bool>     taken   = withTVar(tFork, isForkTaken);
    STML<fp::Unit> newFork = modifyTVar(tFork, setForkTaken);
    STML<bool>     success = sequence(newFork, pure(true));
    STML<bool>     fail    = pure(false);
    STML<bool>     result  = ifThenElse(taken, fail, success);
    return result;
}

STML<bool> takeForks(const TForkPair& forks)
{
    STML<bool> lm = takeFork(forks.left);
    STML<bool> rm = takeFork(forks.right);
    return both<bool, bool, bool>(lm, rm, [](bool l, bool r) { return l && r; });
}

STML<fp::Unit> putFork(const TFork& tFork)
{
    std::function<Fork(Fork)> f = [](const Fork& fork)
    {
        return Fork {fork.name, ForkState::Free};
    };
    return modifyTVar(tFork, f);
}

STML<fp::Unit> putForks(const TForkPair& forks)
{
    STML<fp::Unit> lm = putFork(forks.left);
    STML<fp::Unit> rm = putFork(forks.right);
    return bothVoided(lm, rm);
}

STML<Activity> changeActivity(const Philosopher& philosopher)
{
    STML<Activity> mAct   = readTVar(philosopher.activity);
    STML<Activity> newAct = bind<Activity, Activity>(mAct, [=](Activity act)
        {
            if (act == Activity::Thinking)
            {
                STML<bool>     taken   = takeForks(philosopher.forks);
                STML<fp::Unit> changed = writeTVar(philosopher.activity, Activity::Eating);
                STML<fp::Unit> result  = ifThenElse(taken, changed, mRetry);
                return sequence(result, pure(Activity::Eating));
            }
            else
            {
                STML<fp::Unit> freed   = putForks(philosopher.forks);
                STML<fp::Unit> changed = writeTVar(philosopher.activity, Activity::Thinking);
                STML<fp::Unit> result  = stm::sequence(freed, changed);
                return sequence(result, pure(Activity::Thinking));
            }
        });
    return newAct;
}

const std::function<STML<int>(Philosopher)>
    incrementCycle = [](const Philosopher& philosopher)
{
    return modifyTVarRet(philosopher.cycle, increment);
};

} // namespace philosophers

#endif // PHILOSOPHERS_STM_H
