#include "Containers/include/list.hpp"

#define LOGGING_A
//#define LOGGING_ALLOC

#include <iostream>
#include <memory>
template <typename T>
struct A{
    T value;
    std::unique_ptr<T> ptr;
    inline static int counter = 0;
    inline static int assignments_counter = 0;

    A(): value(T()), ptr(new T(value)){
        ++counter;
#ifdef LOGGING_A
        std::cout
            <<"\n\nA(): " << this
            <<"\n\tptr: " << ptr.get()
            <<"\n\tstatic counter: " << counter; 
        std::cout.flush();
#endif
    }
    

    A(T value) : value(value), ptr(new T(value)) {
        ++counter;
#ifdef LOGGING_A
        std::cout
            <<"\nA (" << value << "): "<< this
            <<"\n\tptr: " << ptr.get() 
            <<"\n\tstatic counter: " << counter; 
        std::cout.flush();
#endif
    }


    A(const A& other): value(other.value), ptr(new T(value)){
        ++counter;
  
        if(counter == 120){
#ifdef LOGGING_A
        std::cout
            <<"\n\t!!! THROW 5; !!!"<< this;
        std::cout.flush();
#endif      
            throw 5;
        }

#ifdef LOGGING_A
        std::cout
            <<"\nA (copy of A(" << other.value << "): "<< this
            <<"\n\tptr: " << ptr.get() 
            <<"\n\tstatic counter: " << counter; 
        std::cout.flush();
#endif
    }
   

    A(A&& other) noexcept: value(std::move(other.value)), ptr(std::move(other.ptr)) {
        ++counter;
#ifdef LOGGING_A
        std::cout
            <<"\nA (move of A(" << other.value << "): "<< this
            <<"\n\tptr: " << ptr.get() 
            <<"\n\tstatic counter: " << counter; 
        std::cout.flush();
#endif
    } 


    A& operator=(const A& other) {
        ++assignments_counter;
 
        if(assignments_counter == 30){
#ifdef LOGGING_A
        std::cout
            <<"\n\t!!! THROW 5; !!!"<< this;
        std::cout.flush();
#endif      
            throw 5;
        }

        value = other.value;
        ptr.reset(new T(value));
       
#ifdef LOGGING_A
        std::cout
            <<"\noperator=(copy of A(" << other.value << ")): "<< this
            <<"\n\tptr: " << ptr.get();
        std::cout.flush();
#endif
       return *this;
    }


    A& operator=(A&& other) noexcept {
        value = std::move(other.value);
        ptr = std::move(other.ptr);
#ifdef LOGGING_A
        std::cout
            <<"\noperator=(move of A(" << other.value << ")): "<< this
            <<"\n\tptr: " << ptr.get();
        std::cout.flush();
#endif
        return *this;
    }


    bool operator==(const A& other) const{
#ifdef LOGGING_A
        std::cout
            <<"\noperator==(const A& other): "<< this
            <<"\n\tptr: " << ptr.get();
        std::cout.flush();
#endif
        return this->value == other.value;
    }

    bool operator!=(const A& other) const{
#ifdef LOGGING_A
        std::cout
            <<"\noperator!=(const A& other): "<< this
            <<"\n\tptr: " << ptr.get();
        std::cout.flush();
#endif
        return !(this->value == other.value);
    }

    ~A(){
        --counter;
#ifdef LOGGING_A
        std::cout
            <<"\n~A() ("<< value <<"): " << this
            <<"\tstatic counter: " << counter; 
        std::cout.flush();
#endif
    }
};


template <typename T>
std::ostream& operator << (std::ostream& os, const A<T>& obj){
    return os << obj.value;
}

template <typename T>
struct CustomAllocator {
    std::string name_; 
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    using propagate_on_container_copy_assignment = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;
    using propagate_on_container_swap = std::true_type;   
    using is_always_equal = std::false_type;
    
    CustomAllocator() noexcept :name_("basic_custom_alloc") {};
    CustomAllocator(std::string name) noexcept :name_(name) {};
    CustomAllocator(const CustomAllocator& other):name_( other.name_ + "(copy)"){};

    template< class U >
    CustomAllocator( const CustomAllocator<U>& other ) noexcept :name_(other.name_ + "(copy from U)"){};


    CustomAllocator& operator=(const CustomAllocator& other)noexcept{
        if(this == &other) return *this;
        name_ = other.name_ + "(copy)";
        return *this;
    };

    template<class U>
    CustomAllocator& operator=(const CustomAllocator<U>& other) noexcept{
         if(this == &other) return *this;
         name_ = other.name_ + "(copy from CustomAllocator<U>)";
         return *this;
    };


    T* allocate(size_t count){
#ifdef LOGGING_ALLOC
        std::cout
            <<"\nallocate(size_t "<< count <<"): name: "<< name_;
        std::cout.flush();
#endif
        
        return static_cast<T*>(operator new(sizeof(T)*count));
    }


    void deallocate(T* ptr, size_t count){ 
#ifdef LOGGING_ALLOC
        std::cout
            <<"\ndeallocate(T* "<< ptr<<", "<< "size_t "<< count <<"): name: "<< name_;
        std::cout.flush();
#endif
        operator delete(ptr);
    }
    

    template <typename U, typename... Args>
    void construct(U* ptr, Args&&... args){
#ifdef LOGGING_ALLOC
        std::cout
            <<"\nconstruct(U* "<< ptr<<", "<< ", Args&&... args): name: "<< name_;
        std::cout.flush();
#endif        
        new(ptr) U(std::forward<Args>(args)...);
    }
 

    template <typename U>
    void destruct(U* ptr){
#ifdef LOGGING_ALLOC
        std::cout
            <<"\ndestruct(U* "<< ptr<<"): name: "<< name_;
        std::cout.flush();
#endif          
        ptr->~U();
    }


    template <typename U>
    struct rebind {
        using other = CustomAllocator<U>;
    };

};

template<typename T, typename U>
constexpr bool operator==(const CustomAllocator<T>&, const CustomAllocator<U>&) noexcept {
    return false;
}

template<typename T, typename U>
constexpr bool operator!=(const CustomAllocator<T>& a, const CustomAllocator<U>& b) noexcept {
    return !(a == b);
}



template <typename Container>
void showList(const Container& container){
    if (container.empty()){std::cout<<"empty";}
    else{
        for (auto& el : container){std::cout<< el <<" ";}
    }
    std::cout<<"\n\tsize: "<<container.size();
    std::cout.flush();
}

int main(){
    try{
        CustomAllocator<A<std::string>> alloc1("alloc1"); 
        CustomAllocator<A<std::string>> alloc2("alloc2"); 
        /*
        std::list<A<std::string>, CustomAllocator<std::string>> l1({
                A{std::string("1")},
                A{std::string("2")},
                A{std::string("3")},
                A{std::string("4")},
                A{std::string("5")},
                A{std::string("6")},
                A{std::string("7")}
                }, alloc1);
        std::list<A<std::string>, CustomAllocator<std::string>> l2({
                A{std::string("20")}, 
                A{std::string("30")}, 
                A{std::string("40")},
                A{std::string("50")}
                }, alloc2);
        */
        List<A<std::string>, CustomAllocator<A<std::string>>> l1({     
            {std::string("1")},
            {std::string("2")},
            {std::string("5")},
            {std::string("4")},
            {std::string("5")},
            {std::string("6")},
            {std::string("5")}
        }, alloc1);

        List<A<std::string>, CustomAllocator<A<std::string>>> l2({
            {std::string("10")},
            {std::string("20")},
            {std::string("30")},
            {std::string("40")},
            {std::string("50")}
        }, alloc2);

        std::cout<<"\n\n\nList1:\n\t";
        showList(l1);

        std::cout<<"\n\nList2: (before assignment)\n\t";
        showList(l2);
        std::cout<<"\n\n";
        
        try{            
            l2.insert(++++l2.begin(), std::make_move_iterator(++l1.begin()), std::make_move_iterator(--l1.end()));          
        }

        catch(...){
            std::cout<<"\n\n\n!!! catched exception in main second level";
            std::cout<<"\n\n\nList2: (after assignment end throw)\n\t";
            showList(l2);
            std::cout<<"\n\n";

            throw;
        }
        std::cout<<"\n\n\nList1: (after assignment)\n\t";
        showList(l1);
        
        std::cout<<"\n\n\nList2: (after assignment)\n\t";
        showList(l2);
        std::cout<<"\n\n";
    }
    catch(...){
        std::cout<<"!!! \n\n\ncatched exception in main first level";
    }
    
    return 0;
}
