#ifndef PHILOSOPHERS_TYPES_H
#define PHILOSOPHERS_TYPES_H

#include <string>
#include <functional>

#include <tvar.h>

namespace philosophers
{

enum class ForkState
{
    Free,
    Taken
};

struct Fork
{
    std::string name;
    ForkState state;
};

enum class Activity
{
    Thinking,
    Eating
};

using TFork = stm::TVar<Fork>;

struct TForkPair
{
    TFork left;
    TFork right;
};

struct ForkPair
{
    Fork left;
    Fork right;
};

const std::function<ForkPair(Fork, Fork)>
mkForkPair =
    [](Fork l, Fork r) { return ForkPair{l, r}; };

struct Philosopher
{
    std::string name;
    stm::TVar<int> cycle;
    stm::TVar<Activity> activity;
    TForkPair forks;
};

std::string printForkState(ForkState state)
{
    switch (state)
    {
    case ForkState::Free: return "Free";
    case ForkState::Taken: return "Taken";
    }
    return "error";
}

std::string printFork(const Fork& fork)
{
    return fork.name + "=" + printForkState(fork.state);
}

std::string printActivity(Activity act)
{
    switch (act)
    {
    case Activity::Eating: return "Eating";
    case Activity::Thinking: return "Thinking";
    }
    return "error";
}

} // namespace philosophers

#endif PHILOSOPHERS_TYPES_H
