//#define LOGGING

#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>

template<typename T, typename Alloc = std::allocator<T>>
class List{
    struct BaseNode{
        BaseNode* prev;
        BaseNode* next;
        BaseNode(BaseNode* prev, BaseNode* next): prev(prev), next(next){}
    };
    
    struct Node: BaseNode{
        T value;
    };
    BaseNode fake_node_;

#if (__cplusplus <=  201402L)
    using NodeAlloc = typename Alloc::template rebind<Node>::other;
#endif
#if (__cplusplus >  201402L)
    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
#endif
    
    NodeAlloc alloc_;
    size_t sz_;
   
public:

    template<bool IsConst = false>
    struct base_iterator{
    private:
        friend class List;
        
        using pointer_type_   = typename std::conditional<IsConst, const T*, T*>::type;
        using refernece_type_ = typename std::conditional<IsConst, const T&, T&>::type;        
        using BaseNode_t = typename std::conditional<IsConst, const BaseNode*, BaseNode*>::type;
        
        BaseNode_t ptr_;
        base_iterator(BaseNode_t ptr):ptr_(ptr){}
        
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type	    = T;
        using pointer           = pointer_type_;
        using reference         = refernece_type_;
        using iterator_category = std::bidirectional_iterator_tag;
        
        
        base_iterator(const base_iterator& other) = default;
        base_iterator& operator=(const base_iterator& other) = default;
        
        refernece_type_ operator*(){
            return reinterpret_cast<Node*>(ptr_)->value;
        }
        pointer_type_ operator->(){
            return reinterpret_cast<Node*>(ptr_);
        }
        
        base_iterator& operator++(){ 
            ptr_ = ptr_->next;
            return *this;
        }
        base_iterator& operator++(int ){ 
            base_iterator temp = *this;
            ptr_ = ptr_->next;
            return temp;
        }
        base_iterator& operator--(){
            ptr_ = ptr_->prev;
            return *this;
        }
        base_iterator& operator--(int ){ 
            base_iterator temp = *this;
            ptr_ = ptr_->prev;
            return temp;
        }
      

        template<bool OtherIsConst>
        bool operator==(const base_iterator<OtherIsConst>& other){
            return ptr_ == other.ptr_;
        }  
        
        operator base_iterator<true>()const{return {ptr_};}
    };

    using value_type	  = T;
    using allocator_type  = Alloc;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference	      = value_type&;
    using const_reference = const value_type&;
    using pointer         = typename std::allocator_traits<Alloc>::pointer;      	 
    using const_pointer   = typename std::allocator_traits<Alloc>::const_pointer;

    using iterator               = base_iterator<false>;	
    using const_iterator         = base_iterator<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() : fake_node_(&fake_node_, &fake_node_), alloc_(), sz_(0){}
    ~List(){clear();}

    List& operator=(const List& other){
        if (this == &other) return other;

        Alloc new_alloc = std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value
            ? other.alloc_ : alloc_;
        
        if(alloc_ == other.alloc_){
            try{
                if (sz_ >= other.sz_){
                    auto this_it = begin();
                    auto other_it = other.cbegin();
                    auto other_end = other.end();
                    for (; other_it != other_end; ++other_it, ++this_it){
                        
                    }
                }
            }
            catch(...){throw;}
        }

        

#ifdef LOGGING
        std::cout<<"operator=(const List& other)" 
            <<"\n\tthis - " << this
            <<"\n\tother - "<< &other 
            <<"\n"; 
#endif    
    }



    reference front(){
        return *begin();
    }
    const_reference front() const {
        return *cbegin();
    }

    reference back(){
        return *(--end());
    }
    const_reference cback() const {
        return *(--cend());
    }
 
    

    iterator begin(){return {fake_node_.next};}
    iterator end(){return &fake_node_;}
    
    const_iterator begin() const {return fake_node_.next;}
    const_iterator end() const {return &fake_node_;}

    const_iterator cbegin() const {return fake_node_.next;}
    const_iterator cend() const {return &fake_node_;}


    /*
    The std::make_reverse_iterator function in C++, when creating a reverse iterator,
    automatically shifts the iterator so that it points to an element that matches the 
    logic of the reverse iterator.
    */
    reverse_iterator rbegin(){return std::make_reverse_iterator(begin());}
    reverse_iterator rend(){return std::make_reverse_iterator(end());}
    
    const_reverse_iterator rbegin() const {return std::make_reverse_iterator(cbegin());}
    const_reverse_iterator rend() const {return std::make_reverse_iterator(cend());}
    
    const_reverse_iterator crbegin() const {return std::make_reverse_iterator(cbegin());}
    const_reverse_iterator crend() const {return std::make_reverse_iterator(cend());}




    bool empty(){return sz_ == 0;}
   
    size_t size(){return sz_;}
    
    long max_size() const{return std::numeric_limits<difference_type>::max();}

 


    void clear(){
        while(!empty()){
            pop_front();
            pop_back();
        }
    }

    //insert
    //emplace
    //erase

    
    void push_back(const T& value){
        Node* new_node_ptr;
        try{
            new_node_ptr = std::allocator_traits<NodeAlloc>::allocate(alloc_, 1);
            try{ // T Node::value construct
                std::allocator_traits<NodeAlloc>::construct(alloc_, &new_node_ptr->value, value);
            }
            catch(...){
                std::allocator_traits<NodeAlloc>::deallocate(alloc_, new_node_ptr, 1);
                throw;
            }
        }
        catch(...){
            throw;
        }
        
        if (empty()){
            new_node_ptr->next = &fake_node_;
            new_node_ptr->prev = &fake_node_;
            fake_node_.next = new_node_ptr;
            fake_node_.prev = new_node_ptr;
        }
        else{
            BaseNode* fake_node_prev = fake_node_.prev;  
            fake_node_prev->next = new_node_ptr;
            new_node_ptr->prev = fake_node_prev;
            new_node_ptr->next = &fake_node_;
            fake_node_.prev = new_node_ptr;
        }
        ++sz_;
#ifdef LOGGING
        std::cout<<"Pushed in back: " << new_node_ptr <<"\n";
#endif    
    }

    //emplace_back()

    void pop_back(){
        if (empty()) {return;}

        Node* delete_node = static_cast<Node*>(fake_node_.prev);
#ifdef LOGGING
        std::cout<<"Popping from back: " << delete_node <<"\n";
#endif
         fake_node_.prev = delete_node->prev;
        fake_node_.prev->next = &fake_node_; 
        std::allocator_traits<NodeAlloc>::destroy(alloc_, delete_node);
        std::allocator_traits<NodeAlloc>::deallocate(alloc_, delete_node, 1);
        --sz_;
#ifdef LOGGING
        std::cout<<"Popped from back: " << delete_node <<"\n";
#endif
     }
    



    void push_front(const T& value){
        if (empty()){
            push_back(value);
        }
    
        Node* new_node_ptr;
        try{
            new_node_ptr = std::allocator_traits<Alloc>::allocate(alloc_, 1);
            try{
                std::allocator_traits<Alloc>::construct(alloc_, &new_node_ptr->value, value);
            }
            catch(...){
                std::allocator_traits<Alloc>::deallocate(alloc_, new_node_ptr, 1);
                throw;
            }
        }
        catch(...){
            throw;
        }

        Node* fake_node_next = fake_node_->next;  
        fake_node_->next = new_node_ptr;
        new_node_ptr->prev = fake_node_;
        new_node_ptr->next = fake_node_next;           
        fake_node_next->prev = new_node_ptr;
        ++sz_;
#ifdef LOGGING
        std::cout<<"Pushed in front: " << new_node_ptr <<"\n";
#endif
        
    }


    //emplace_front()


    void pop_front(){
        if (empty()) {return;}

        Node* delete_node = static_cast<Node*>(fake_node_.next);
#ifdef LOGGING
        std::cout<<"Popping from front: " << delete_node <<"\n";
#endif
        fake_node_.next = delete_node->next;
        fake_node_.next->prev = &fake_node_; 
        std::allocator_traits<NodeAlloc>::destroy(alloc_, delete_node);
        std::allocator_traits<NodeAlloc>::deallocate(alloc_, delete_node, 1);
        --sz_;
#ifdef LOGGING
        std::cout<<"Popped from front: " << delete_node <<"\n";
#endif
    }

   
    //resize
   
   
    //swap
   
   
    //merge
    
    
    //splice
    
    
    //remove
    

    //remove_if


    //unique

    //sort
   
};


struct A{
    int a;
    A(){
#ifdef LOGGING

        std::cout<<"A(): " << this << "\n";
#endif
    }
    
    A(int a) : a(a) {
#ifdef LOGGING
        std::cout<<"A(" << a << "): "<< this << "\n";
#endif
    }

    A(const A& other): a(other.a){
#ifdef LOGGING
        std::cout<<"A(copy of A(" << other.a << ")): "<< this << "\n";
#endif
    }
    
    A& operator=(const A& other) {
        a = other.a;
#ifdef LOGGING
        std::cout<<"operator=(copy of A(" << other.a << ")): "<< this << "\n";
#endif
        return *this;
    }

    ~A(){
#ifdef LOGGING
        std::cout<<"~A(): " << this << "\n";
#endif
    }
};

int main(){
    
    return 0;
}
