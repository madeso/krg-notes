#include <optional>
#include <vector>
#include <memory>
#include <iostream>
#include <string>



enum class State
{
    Running, Failure, Success
};

struct Node
{
    std::optional<State> state;

    State Update(float dt)
    {
        if(!state)
        {
            OnStart();
        }

        const auto current_state = OnUpdate(dt);

        if(current_state != State::Running)
        {
            OnStop();
            state = std::nullopt;
        }
        else
        {
            state = current_state;
        }

        return current_state;
    }

    virtual void OnStart() = 0;
    virtual void OnStop() = 0;
    virtual State OnUpdate(float dt) = 0;
};

struct ActionNode : Node {};

struct DecoratorNode : Node
{
    std::shared_ptr<Node> child;
};

struct CompositeNode : Node
{
    std::vector<std::shared_ptr<Node>> children;
};

//

struct LogNode : ActionNode
{
    std::string message;

    void OnStart()
    {
        std::cout << "OnStart:" << message << '\n';
    }

    void OnStop()
    {
        std::cout << "OnStop:" << message << '\n';
    }

    State OnUpdate(float)
    {
        std::cout << "OnUpdate:" << message << '\n';
    }
};

struct RepeatNode : DecoratorNode
{
    void OnStart() { }
    void OnStop() { }

    State OnUpdate(float dt)
    {
        child->Update(dt);
        return State::Running;
    }
};

struct SequencerNode : CompositeNode
{
    int current = 0;

    void OnStart()
    {
        current = 0;
    }

    void OnStop()
    {
    }

    State OnUpdate(float dt)
    {
        const auto state = children[current]->Update(dt);
        if(state != State::Success) { return state; }
        
        current += 1;
        if(current < children.size())
        {
            return State::Running;
        }
        else
        {
            return State::Success;
        }
    }
};

struct WaitNode : ActionNode
{
    float duration = 0.0f;
    float timer = 0.0f;


    void OnStart()
    {
        timer = 0.0f;
    }

    void OnStop()
    {
    }

    State OnUpdate(float dt)
    {
        timer += dt;

        if(timer >= duration)
        {
            return State::Success;
        }
        else
        {
            return State::Running;
        }
    }
};

//

struct BhTree
{
    std::shared_ptr<Node> root;
    State state = State::Running;

    State Update(float dt)
    {
        if(state == State::Running)
        {
            state = root->Update(dt);
        }

        return state;
    }
};

void demo()
{
    auto tree = BhTree{};
    auto log = std::make_shared<LogNode>();
    log->message = "hello world";

    tree.root = log;
}

struct Runner
{
};
