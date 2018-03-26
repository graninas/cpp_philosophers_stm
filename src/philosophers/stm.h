#ifndef STM_H
#define STM_H

#include <functional>
#include <any>

#include <stm.h>

#include "types.h"

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
    STML<Fork> f = readFork(tFork);
    return bind(f, [=](const Fork& fork)
    {
        if (fork.state == ForkState::Taken)
        {
            return pure(false);
        }
        else
        {
            auto m = writeTVar(tFork, {fork.name, ForkState::Taken});
            return bind(m, [](auto){ return pure(true);});
        }
    });
};

const std::function<STML<ForkPair>(TForkPair)>
    takeForks =
        [](const TForkPair& forks)
{
    STML<bool> lm = takeFork(forks.left);
    STML<bool> rm = takeFork(forks.right);
    return both(lm, rm, [](bool l, bool r) { return l && r; });
};

const std::function<STML<fp::Unit>(TFork)>
    putFork =
        [](const TFork& tFork)
{
    return modifyTVar(tFork,
                      [](const Fork& fork)
    {
        return Fork {fork.name, ForkState::Free};
    } );
};

const std::function<STML<fp::Unit>(TForkPair)>
    putForks =
        [](const TForkPair& forks)
{
    STML<fp::Unit> lm = putFork(forks.left);
    STML<fp::Unit> rm = putFork(forks.right);
    return bothVoided(lm, rm);
};

//const std::function<STML<Activity>(Philosopher)>
//    changeActivity =

} // namespace philosophers

//changeActivity :: Philosopher -> STML Activity
//changeActivity (Philosopher n tC tAct tFs) = do
//  act <- readTVar tAct
//  case act of
//    Thinking -> do
//      taken <- takeForks tFs
//      unless taken retry  -- Do not need to put forks if any was taken!
//      writeTVar tAct Eating
//      pure Eating
//    Eating -> do
//      putForks tFs
//      writeTVar tAct Thinking
//      pure Thinking

//incrementCycles :: Philosopher -> STML Int
//incrementCycles (Philosopher _ tCycles _ _) = do
//  modifyTVar tCycles (+1)
//  readTVar tCycles

#endif // STM_H
