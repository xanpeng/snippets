#include <iostream>
#include <boost/smart_ptr.hpp>

class Foo {
public:
    Foo() { std::cout << "Foo()" << std::endl; }
    ~Foo() { std::cout << "~Foo()" << std::endl; }
    void Show() { std::cout << "Hello, I am Foo" << std::endl; }
    bool False() { return false; }
};

void ScopedPtrSample() {
    boost::scoped_ptr<Foo> foo_ptr(new Foo);

    if (!foo_ptr->False())
        return;

    foo_ptr->Show();
}

void NonScopedPtrSample() {
    Foo* pfoo(new Foo);

    if (!pfoo->False()) {
        delete pfoo;
        return;
    }

    pfoo->Show();
    delete pfoo;
}

int main() {
    // NonScopedPtrSample();
    ScopedPtrSample();

    return 0;
}
