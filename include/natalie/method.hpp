#pragma once

#include "natalie/block.hpp"
#include "natalie/env.hpp"
#include "natalie/forward.hpp"
#include "natalie/gc.hpp"
#include "natalie/method_visibility.hpp"
#include "natalie/module_value.hpp"

namespace Natalie {

class Method : public Cell {
public:
    Method(const char *name, ModuleValue *owner, MethodFnPtr fn, int arity, MethodVisibility visibility)
        : m_name { name }
        , m_owner { owner }
        , m_fn { fn }
        , m_arity { arity }
        , m_undefined { !fn }
        , m_visibility { visibility } { }

    Method(const char *name, ModuleValue *owner, Block *block, MethodVisibility visibility)
        : m_name { name }
        , m_owner { owner }
        , m_arity { block->arity() }
        , m_env { *block->env() }
        , m_visibility { visibility } {
        block->copy_fn_pointer_to_method(this);
        m_env.clear_caller();
    }

    void set_fn(MethodFnPtr fn) { m_fn = fn; }

    Env *env() { return &m_env; }
    bool has_env() { return !!m_env.global_env(); }

    bool is_undefined() { return m_undefined; }

    ValuePtr call(Env *env, ValuePtr self, size_t argc, ValuePtr *args, Block *block) {
        assert(!is_undefined());
        Env *closure_env;
        if (has_env()) {
            closure_env = this->env();
        } else {
            closure_env = m_owner->env();
        }
        auto *e = new Env { closure_env };
        e->set_caller(env);
        e->set_method(this);
        e->set_file(env->file());
        e->set_line(env->line());
        e->set_block(block);
        auto result = m_fn(e, self, argc, args, block);
        e->clear_caller();
        return result;
    }

    const String *name() { return &m_name; }
    ModuleValue *owner() { return m_owner; }

    MethodVisibility visibility() { return m_visibility; }

    int arity() { return m_arity; }

    virtual void visit_children(Visitor &visitor) override final {
        visitor.visit(m_owner);
        m_env.visit_children(visitor); // must call visit_children directly since we own this object
    }

    virtual void gc_print() override {
        fprintf(stderr, "<Method %p name='%s' fn=%p>", this, m_name.c_str(), m_fn);
    }

private:
    String m_name {};
    ModuleValue *m_owner;
    MethodFnPtr m_fn;
    int m_arity { 0 };
    Env m_env {};
    bool m_undefined { false };
    MethodVisibility m_visibility { MethodVisibility::Public };
};
}
