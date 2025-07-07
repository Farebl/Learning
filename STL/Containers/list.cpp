//#define LOGGING

#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>

template<typename T, typename Allocator = std::allocator<T>>
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
    using NodeAllocator = typename Allocator::template rebind<Node>::other;
#endif
#if (__cplusplus >  201402L)
    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
#endif
    
    NodeAllocator alloc_;
    size_t sz_;
   
public:

    template<bool IsConst = false>
    struct base_iterator{
    private:
        friend class List;
        
        using pointer_type_   = typename std::conditional<IsConst, const T*, T*>::type;
        using refernece_type_ = typename std::conditional<IsConst, const T&, T&>::type;        
        using BaseNodePtr_t = typename std::conditional<IsConst, const BaseNode*, BaseNode*>::type;
        using NodePtr_t = typename std::conditional<IsConst, const Node*, Node*>::type;
        
        BaseNodePtr_t ptr_;
        base_iterator(BaseNodePtr_t ptr):ptr_(ptr){}
    public:

        using difference_type   = std::ptrdiff_t;
        using value_type	    = T;
        using pointer           = pointer_type_;
        using reference         = refernece_type_;
        using iterator_category = std::bidirectional_iterator_tag;
        
        
        base_iterator(const base_iterator& other) = default;
        base_iterator& operator=(const base_iterator& other) = default;
        
        refernece_type_ operator*(){
            return static_cast<NodePtr_t>(ptr_)->value;
        }
        
        pointer_type_ operator->(){
            return &static_cast<NodePtr_t>(ptr_)->value;;
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
        template<bool OtherIsConst>
        bool operator!=(const base_iterator<OtherIsConst>& other){
            return !(ptr_ == other.ptr_);
        }
        
        operator base_iterator<true>() const {return {ptr_};}
    };

    using value_type	  = T;
    using allocator_type  = Allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference	      = value_type&;
    using const_reference = const value_type&;
    using pointer         = typename std::allocator_traits<Allocator>::pointer;      	 
    using const_pointer   = typename std::allocator_traits<Allocator>::const_pointer;

    using iterator               = base_iterator<false>;	
    using const_iterator         = base_iterator<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    
    List() : fake_node_(&fake_node_, &fake_node_), alloc_(), sz_(0){
#ifdef LOGGING
        std::cout<<"\n\nList(): " << this;
        std::cout.flush();
#endif 
    }
    
    //List(const Allocator& alloc){}
    
    //List(size_t count, const Allocator& alloc){}
    
    //List(size_t count, const T& value, const Allocator& alloc){}
    
    //template <typename InputIt>
    //List(InputIt first, InputIt last, const Allocator& alloc){}


    List(const List& other) 
        : fake_node_(&fake_node_, &fake_node_) 
        , alloc_(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator()))
        , sz_(0)
    {
#ifdef LOGGING
        std::cout<<"\n\nList(const List& other): " << this;
        std::cout.flush();
#endif 
        try{
            auto this_it = begin();
            auto other_it = other.begin();
            auto other_end = other.end();
    //        for()
  //          other.sz_;
        }
        catch(...){
            
            throw;
        }
        
    
    }
    
    //List(const List& other, const Allocator& alloc){}
    
    //List(std::initializer_list<T> init_list, const Allocator& alloc){}

    
    ~List() {
        clear();
#ifdef LOGGING
        std::cout<<"\n\n~List(): " << this<<"\n";
        std::cout.flush();
#endif
 
    }

    List& operator=(const List& other){
        if (this == &other) return other;
        
        Allocator new_alloc = std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value
            ? other.alloc_ : alloc_;
        
        if(alloc_ == other.alloc_){
            try{
                if (sz_ >= other.sz_){
                    auto this_it = begin();
                    auto other_it = other.cbegin();
                    auto other_end = other.end();
                    for (; other_it != other_end; ++other_it, ++this_it){
                        *this_it = *other_it;
                    }

                }
            }
            catch(...){throw;}
        }

        

#ifdef LOGGING
        std::cout<<"\n\noperator=(const List& other)" 
            <<"\n\tthis  - " << this
            <<"\n\tother - " << &other;
        std::cout.flush();
#endif    
    }

    // List& operator=(std::initializer_list<T> ilist){}

    //void assign(size_t count, const T& value){}
    
    
    //template< class InputIt>
    //void assign(InputIt first, InputIt last){}

    //void assign(std::initializer_list<T> init_list){}
    
    Allocator get_allocator(){return alloc_;}

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
    iterator end(){return {&fake_node_};}
    
    const_iterator begin() const {return fake_node_.next;}
    const_iterator end() const {return {&fake_node_};}

    const_iterator cbegin() const {return {fake_node_.next};}
    const_iterator cend() const {return {&fake_node_};}


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
    


    iterator insert(const_iterator pos, const T& value){
        Node* new_node = nullptr; 
        try{
            new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
            try{
                std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, value);
            }
            catch(...){throw;}    
        }
        catch(...){
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, new_node, 1);
            throw;
        }
        /*
            Using of the const_cast<T*>(const T*) there is never UB here, 
            because at the memory level all elements are non-constant.
        */  
        BaseNode* pos_node = const_cast<BaseNode*>(pos.ptr_);
        
        new_node->next = pos_node->next;
        new_node->next->prev = new_node;
        
        pos_node->next = new_node;
        new_node->prev = pos_node;

        ++sz_;
#ifdef LOGGING
        std::cout
            <<"\n\nInserted in : " << new_node
            <<"\n\tprev:       " << new_node->prev 
            <<"\n\tnext:       " << new_node->next
            <<"\n\tsize:       " << sz_;
        std::cout.flush();
#endif        

        return {new_node};
    }

    

    iterator insert( const_iterator pos, size_type count, const T& value ){
#ifdef LOGGING
        std::cout<<"\n\nStart inserting "<< count << " new elements:";
        std::cout.flush();
#endif          

        Node* first_new_node = nullptr;
        size_t inserted_count = 0;
        try{
            first_new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
            try{
                std::allocator_traits<NodeAllocator>::construct(alloc_, &first_new_node->value, value);
            }
            catch(...){throw;} 
        }
        catch(...){
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, first_new_node, 1);
            throw;
        }
        
       ++inserted_count;
#ifdef LOGGING
        std::cout<<"\n\t created " << inserted_count <<"/"<< count << " element: " << first_new_node;
        std::cout.flush();
#endif        


        Node* last_new_node = first_new_node;
        try{
            for(; inserted_count != count; ++inserted_count){

                last_new_node->next = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
                try{
                    /*The Node::prev and Node::next elements are of type (BaseNode*).*/
                    std::allocator_traits<NodeAllocator>::construct(alloc_, &static_cast<Node*>(last_new_node->next)->value, value);
                }
                catch(...){throw;}
                last_new_node->next->prev = last_new_node;
                last_new_node = static_cast<Node*>(last_new_node->next);
#ifdef LOGGING
                std::cout<<"\n\t created " << inserted_count+1 <<"/"<< count << " element: " << last_new_node<<std::endl;
                std::cout.flush();
#endif        
            }
        }
        catch(...){
            while(last_new_node != first_new_node){
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(last_new_node->next));
                /*The Node::prev and Node::next elements are of type (BaseNode*).*/
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(last_new_node->next), 1);
                
                last_new_node = static_cast<Node*>(last_new_node->prev);
            }
            //Removing the second new node
            std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(first_new_node->next));
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(first_new_node->next), 1); 
            
            //Removing the first new node
            std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(first_new_node));
            std::allocator_traits<NodeAllocator>::deallocate(alloc_,static_cast<Node*>( first_new_node), 1); 
            throw;
        }

    
        /*
            Using of the const_cast<T*>(const T*) there is never UB here, 
            because at the memory level all elements are non-constant.
        */  
        BaseNode* pos_node = const_cast<BaseNode*>(pos.ptr_);
    

        last_new_node->next = pos_node->next;
        last_new_node->next->prev = last_new_node;
        
        pos_node->next = first_new_node;
        first_new_node->prev = pos_node;

        sz_+=inserted_count;
#ifdef LOGGING
        std::cout<<"\n\nEnd inserting "<< count << " new elements:"
            <<"\nInserted in   "
            <<"\nfirst - last: " << first_new_node <<" - " << last_new_node
            <<"\n\tfist_prev:  " << first_new_node->prev 
            <<"\n\tlast_next:  " << last_new_node->next
            <<"\n\tsize:       " << sz_;
        std::cout.flush();
#endif        

        return {first_new_node};
    }   

    //template< class InputIt > 
    //iterator insert( const_iterator pos, InputIt first, InputIt last );
    
    //iterator insert( const_iterator pos, std::initializer_list<T> ilist );

    
    
    iterator erase(const_iterator pos){
        if (pos == end()) return end();
        
        const BaseNode* delete_node = pos.ptr_; 
        delete_node->prev->next = delete_node->next;
        delete_node->next->prev = delete_node->prev;
            
        BaseNode* delete_node_next = delete_node->next;
        
        std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(const_cast<BaseNode*>(delete_node)));
        /*
            Using of the const_cast<T*>(const T*) there is never UB here, 
            because at the memory level all elements are non-constant.
        */
        std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(const_cast<BaseNode*>(delete_node)), 1);
        
        --sz_;

#ifdef LOGGING
        std::cout<<"\n\nerased: " << delete_node;
        std::cout.flush();
#endif       

        return {delete_node_next};
    }


    iterator erase(const_iterator first, const_iterator last){
        if(first == last) return const_cast<BaseNode*>(last.ptr_);
        
        /* 
            The BaseNode::next and BaseNode::prev fields are non-constant, 
            but the const_iterator::ptr_ pointer is const 
        */
        BaseNode* first_prev = first.ptr_->prev;  
        iterator following_after_delete = first.ptr_->next;

#ifdef LOGGING
        std::cout<<"\n\nStart erasing: [" << first.ptr_ << " ; " << last.ptr_ << ")";
        std::cout.flush();
#endif       
        if(last != cend()){
             while (following_after_delete.ptr_->prev != last.ptr_) { 
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(following_after_delete.ptr_->prev));
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(following_after_delete.ptr_->prev), 1);    
        
                --sz_;
#ifdef LOGGING
                std::cout
                    <<"\n\terased:           " << following_after_delete.ptr_->prev
                    <<"\n\tnext_value:       " << *following_after_delete
                    <<"\n\tsize:             " << sz_
                    <<"\n\tnext_dell - last: " <<following_after_delete.ptr_ <<" - "<< last.ptr_;
                std::cout.flush();
#endif        
                ++following_after_delete;
            }

#ifdef LOGGING
            std::cout
                <<"\n\nend erasing:        "
                <<"\n\tfollowing node      "
                <<"\n\tafter last deleted: " << following_after_delete.ptr_
                <<"\n\tfirst_prev:         " << first_prev;
            std::cout.flush();
#endif        

            first_prev->next = following_after_delete.ptr_->prev;
            following_after_delete.ptr_->prev->prev = first_prev;
        }

        else{ 
            while (following_after_delete.ptr_->prev != last.ptr_) { 
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(following_after_delete.ptr_->prev));
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(following_after_delete.ptr_->prev), 1);    
        
                --sz_;
#ifdef LOGGING
                std::cout
                    <<"\n\terased:           " << following_after_delete.ptr_->prev
                    <<"\n\tnext_value:       " << *following_after_delete
                    <<"\n\tsize:             " << sz_
                    <<"\n\tnext_dell - last: " <<following_after_delete.ptr_ <<" - "<< last.ptr_;
                std::cout.flush();
#endif        
                if (following_after_delete.ptr_ != &fake_node_)
                    ++following_after_delete;
                else{
                    break;
                }
            }
#ifdef LOGGING
            std::cout
                <<"\n\nend erasing:        "
                <<"\n\tfollowing node      "
                <<"\n\tafter last deleted: " << following_after_delete.ptr_
                <<"\n\tfirst_prev:         " << first_prev;
            std::cout.flush();
#endif        

            first_prev->next = following_after_delete.ptr_;
            following_after_delete.ptr_->prev->prev = first_prev;
        }

        return {following_after_delete.ptr_};
    }

    /*
    versions of these methods for non-constant iterators:
        
        iterator erase(iterator first, iterator last){...}
        iterator erase(iterator pos){...}

    are unnecessary, because there is an implicit conversion 
    of a non-constant iterator to a constant one:
        
        operator base_iterator<true>() const {return {ptr_};}
    */



    void push_back(const T& value){
#ifdef LOGGING
        std::cout<<"\n\nPushing back: " << &value;
        std::cout.flush();
#endif
        Node* new_node;
        try{
            new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
            try{ // T Node::value construct
                std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, value);
            }
            catch(...){throw;}
        }
        catch(...){
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, new_node, 1);
            throw;
        }
        BaseNode* fake_node_prev = fake_node_.prev;  
        fake_node_prev->next = new_node;
        new_node->prev = fake_node_prev;
        new_node->next = &fake_node_;
        fake_node_.prev = new_node;
        ++sz_;

#ifdef LOGGING
        std::cout
            <<"\n\nPushed in back: " 
            <<"\n\tnew node:  " << new_node
            <<"\n\tprev node: " << new_node->prev
            <<"\n\tnew size:  " << sz_;
        std::cout.flush();
#endif    
    }


    void pop_back(){
        if (empty()) {return;}

        BaseNode* delete_node = fake_node_.prev;
#ifdef LOGGING
        std::cout<<"\n\nPopping from back: " << delete_node;
        std::cout.flush();
#endif
        fake_node_.prev = delete_node->prev;
        fake_node_.prev->next = &fake_node_; 
        std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(delete_node));
        std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(delete_node), 1);
        --sz_;
#ifdef LOGGING
        std::cout
            <<"\nPopped from back: " 
            <<"\n\tdeleted node: " << delete_node
            <<"\n\tnew last:    " << fake_node_.prev  
            <<"\n\tnew size:     " << sz_; 
        std::cout.flush();
#endif        
    }
    


    void push_front(const T& value){
#ifdef LOGGING
        std::cout<<"\n\nPushing front: " << &value;
        std::cout.flush();
#endif
        if (empty()){
            push_back(value);
        }
    
        Node* new_node;
        try{
            new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
            try{
                std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, value);
            }
            catch(...){throw;}
        }
        catch(...){
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, new_node, 1);
            throw;
        }

        BaseNode* fake_node_next = fake_node_.next;  
        fake_node_.next = new_node;
        new_node->prev = &fake_node_;
        new_node->next = fake_node_next;           
        fake_node_next->prev = new_node;
        ++sz_;

#ifdef LOGGING
        std::cout
            <<"\n\nPushed in front: " 
            <<"\n\tnew node:  " << new_node
            <<"\n\tnext node: " << new_node->next
            <<"\n\tnew size:  " << sz_;
        std::cout.flush();
#endif    
        
    }



    void pop_front(){
        if (empty()) {return;}

        BaseNode* delete_node = fake_node_.next;
#ifdef LOGGING
        std::cout<<"\n\nPopping from front: " << delete_node;
        std::cout.flush();
#endif
        fake_node_.next = delete_node->next;
        fake_node_.next->prev = &fake_node_; 
        std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(delete_node));
        std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(delete_node), 1);
        --sz_;
#ifdef LOGGING
        std::cout
            <<"\n\nPopped from front: " 
            <<"\n\tdeleted node: " << delete_node
            <<"\n\tnew begin:    " << fake_node_.next  
            <<"\n\tnew size:     " << sz_; 
        std::cout.flush();
#endif    
    }

   
    //void resize( size_type count );
    
    //void resize( size_type count, const value_type& value );   
   
   
    //void swap( list& other ) noexcept{}
   
   
    //merge
    
    
    //splice
    
    
    //remove
    

    //remove_if


    //unique

    //sort
   
};



struct A{
    int a;
    inline static int val = 0;
    A(){
        ++val;
#ifdef LOGGING
        std::cout
            <<"\n\nA(): " << this
            <<"\tstatic val:" << val; 
        std::cout.flush();
#endif
    }
    
    A(int a) : a(a) {
        ++val;
#ifdef LOGGING
        std::cout
            <<"\n\nA (" << a << "): "<< this
            <<"\tstatic val:" << val; 
        std::cout.flush();
#endif
    }

    A(const A& other): a(other.a){
        ++val;
#ifdef LOGGING
        std::cout
            <<"\n\nA (copy of A" << other.a << "): "<< this
            <<"\tstatic val:" << val; 
        std::cout.flush();
#endif
        if(val == 5){
#ifdef LOGGING
        std::cout
            <<"\n\t!!! THROW 5; !!!"<< this;
        std::cout.flush();
#endif
            
            throw 5;
        }
    }
    
    A& operator=(const A& other) {
        a = other.a;
#ifdef LOGGING
        std::cout<<"\n\noperator=(copy of A(" << other.a << ")): "<< this;
        std::cout.flush();
#endif
        return *this;
    }

    ~A(){
        --val;
#ifdef LOGGING
        std::cout
            <<"\n\n~A() ("<< a <<"): " << this
            <<"\tstatic val:" << val; 
        std::cout.flush();
#endif
    }
};

std::ostream& operator<< (std::ostream& os, const A& obj){
    return os << obj.a;
}

template <typename T, typename Allocator>
void showList(const List<T, Allocator>& list){
    for (auto& el : list){std::cout<< el <<" ";}
    std::cout.flush();
}

int main(){
    
    return 0;
}
