#include <iostream>
#include <new>


template <typename T, typename Deleter>
class SmartPtr {
public:
 SmartPtr(T *ptr, Deleter del): ptr_{ptr}, del_{del}{}
 ~SmartPtr() {del_(ptr);}
 T* operator*() { return ptr_;}
 const T* operator*() const { return ptr_;}


private:
 T* ptr_;
 Deleter del_;
};

template <typename T>
class SmartPtrTE {
 struct DeleteBase {
    virtual void apply(void*) = 0;
    virtual ~DeleteBase() = 0;
 }
 template <typename D>
 struct Delete : public DeleteBase {
    Delete(D d): del_{d} {}
    void apply(void *d) override {
        del_(static_cast<D*>(d));
    }
    ~Deleter() {}
    T del_;
 }
 public:
 template <typename D>
 SmartPtrTE(T* t, D d): ptr_{ptr}, del{ new Delete<D> {d}}{}
 ~SmartPtrTE() {
    del->apply(ptr_);
    delete del_;
 }
 T* operator*() { return ptr_;}
 const T* operator*() const { return ptr_;}
 private:
 T* ptr_;
 DeleteBase* del_;

}; 