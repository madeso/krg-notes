#include <array>
#include <memory>
#include <optional>
#include <cassert>

template<typename Enum, typename Context, std::size_t count>
struct Machine;

template<typename Enum, typename Context, std::size_t count>
struct Node
{
    Node() = default;
    virtual ~Node() = default;
    virtual void update(Context* c, Machine<Enum, Context, count>* m) const = 0;
};

template<typename Enum, typename Context, std::size_t count>
struct Machine
{
    using NodeT = Node<Enum, Context, count>;
    
    std::array<std::unique_ptr<NodeT>, count> nodes;
    std::optional<Enum> current_node;

    void change_node(Enum id)
    {
        const auto iid = static_cast<std::size_t>(id);
        assert(iid < count);
        current_node = id;
    }

    void add_node(Enum id, std::unique_ptr<NodeT>&& node)
    {
        const auto iid = static_cast<std::size_t>(id);
        assert(iid < count);
        nodes[iid] = std::move(node);
    }

    void update(Context* c)
    {
        if(!current_node) return;
        auto& node = nodes[static_cast<std::size_t>(*current_node)];
        node->update(c, this);
    }
};


struct Character
{
    int state = 0;
};

enum class State { Initial, Next, COUNT };

using TNode = Node<State, Character, static_cast<std::size_t>(State::COUNT)>;
using TMachine = Machine<State, Character, static_cast<std::size_t>(State::COUNT)>;

struct Initial : TNode
{
    Initial() = default;
    void update(Character* c, TMachine* m) const override
    {
        m->change_node(State::Next);
    }
};

struct Next : TNode
{
    void update(Character* c, TMachine* m) const override
    {
        c->state += 1;

        if((c->state%2) == 0) {
            m->change_node(State::Initial);
        }
    }
};

void test()
{
    TMachine m;
    m.add_node(State::Initial, std::make_unique<Initial>());
    m.add_node(State::Next, std::make_unique<Next>());

    Character c;
    for(int i=0; i<100; i+=1)
    {
        m.update(&c);
    }
}

