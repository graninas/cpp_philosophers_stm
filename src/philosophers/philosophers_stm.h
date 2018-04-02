#ifndef PHILOSOPHERS_STM_H
#define PHILOSOPHERS_STM_H

#include <iostream>

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
//    std::cout << "Philosopher " << philosopher.name << ": trying to change activity." << std::endl;
    STML<Activity> mAct   = readTVar(philosopher.activity);
    STML<Activity> newAct = bind<Activity, Activity>(mAct, [=](Activity act)
        {
            if (act == Activity::Thinking)
            {
//                std::cout << "Philosopher " << philosopher.name << ": was Thinking" << std::endl;
                STML<bool>     m1 = takeForks(philosopher.forks);
                STML<fp::Unit> m2 = writeTVar(philosopher.activity, Activity::Eating);

//              Twice taking forks!!!
//                STML<fp::Unit> m3 = stm::when(m1, m2);
//                STML<fp::Unit> m4 = stm::unless(m1, stm::mRetry);

                STML<fp::Unit> m3 = stm::conditional(m1, m2, stm::mRetry);
                return sequence(m3, pure(Activity::Eating));
            }
            else
            {
//                std::cout << "Philosopher " << philosopher.name << ": was Eating" << std::endl;
                STML<fp::Unit> m1 = putForks(philosopher.forks);
                STML<fp::Unit> m2 = writeTVar(philosopher.activity, Activity::Thinking);
                STML<fp::Unit> m3 = stm::sequence(m1, m2);
                return sequence(m3, pure(Activity::Thinking));
            }
        });
    return newAct;
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
