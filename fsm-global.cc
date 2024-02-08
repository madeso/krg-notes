/*
    A State machine based on global states

    Issues?:
        * Reference actual types and that is a problem if we want to create a specialization
        * All states must be a class, can't use generic state that have arguments
        * All state but be provided on the context

*/

// ---------------------------------------------------------------------------
// fwd header
template<typename Context>
struct State;
#define DEF_STATE_REF(Context, Name) State<Context>* get_##Name();
#define IMP_STATE_REF(Context, Name) State<Context>* get_##Name() { static Name instance; return &instance;}


// ---------------------------------------------------------------------------
// test header
/// could be a character, ai etc...
struct Test {
    int state = 0;
};
DEF_STATE_REF(Test, StartState)
DEF_STATE_REF(Test, NextState)


// ---------------------------------------------------------------------------
// implementation header

template<typename Context>
struct Machine;

template<typename Context>
struct State
{
    virtual void enter() const {}
    virtual void exit() const {}

    virtual void update(Context*,Machine<Context>*) const = 0;

    protected:
    State() = default;
};

template<typename Context>
struct Machine
{
    const State<Context>* current_state = nullptr;
    const State<Context>* next_state = nullptr;

    void change_state(const State<Context>* n)
    {
        next_state = n;
    }

    void update(Context* c)
    {
        if(current_state) current_state->update(c, this);
        transition_to_next_state();
    }

    void transition_to_next_state()
    {
        if(next_state == current_state) return;
        if(next_state == nullptr) return;

        if(current_state) current_state->exit();

        current_state = next_state;
        next_state = nullptr;

        current_state->enter();
    }
};

// ---------------------------------------------------------------------------
// test cc file

struct NextState : State<Test>
{
    void update(Test* t, Machine<Test>* s) const override
    {
        if((t->state++) % 2 == 1)
        {
            s->change_state(get_StartState());
        }
    }
};
IMP_STATE_REF(Test, NextState)

struct StartState : State<Test>
{
    void update(Test*, Machine<Test>* s) const override
    {
        s->change_state(get_NextState());
    }
};
IMP_STATE_REF(Test, StartState)

void test()
{
    Test test;
    Machine<Test> m;
    m.change_state(get_StartState());

    for(int i = 0; i<100; i+=1)
        m.update(&test);
}

