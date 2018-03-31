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

//const std::function<STML<fp::Unit>(TFork)>
//    putFork =
//        [](const TFork& tFork)
//{
//    return modifyTVar(tFork,
//                      [](const Fork& fork)
//    {
//        return Fork {fork.name, ForkState::Free};
//    } );
//};

//const std::function<STML<fp::Unit>(TForkPair)>
//    putForks =
//        [](const TForkPair& forks)
//{
//    STML<fp::Unit> lm = putFork(forks.left);
//    STML<fp::Unit> rm = putFork(forks.right);
//    return bothVoided(lm, rm);
//};

//const std::function<STML<Activity>(Philosopher)>
//    changeActivity = [](const Philosopher& philosopher)
//{
//    STML<Activity> mAct = readTVar(philosopher.activity);
//    return bind(mAct, [&](Activity activity)
//    {
//        switch (activity) {
//        case Activity::Thinking:
//            STML<bool> mTaken = takeForks(philosopher.forks);
//            STML<fp::Unit> mNext1 = unless(mTaken, retry);
//            STML<fp::Unit> mNext2 = bind(mNext1, [&](auto)
//            {
//                return writeTVar(philosopher.activity, Activity::Eating);
//            });
//            return voided(mNext2, pure(Activity::Eating));

//        case Activity::Eating:
//            STML<fp::Unit> ma = putForks(philosopher.forks);
//            STML<fp::Unit> mb = voided(ma, writeTVar(philosopher.activity, Activity::Thinking));
//            return voided(mb, pure(Activity::Thinking));
//        };
//    });
//};

//const std::function<STML<int>(Philosopher)>
//    incrementCycles = [](const Philosopher& philosopher)
//{
//    STML<fp::Unit> ma = modifyTVar(philosopher.cycles, [](int i) { return i + 1; });
//    return voided(ma, readTVar(philosopher.cycles));
//};

} // namespace philosophers

#endif // PHILOSOPHERS_STM_H
