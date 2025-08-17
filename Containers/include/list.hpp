#ifndef FAREBL_LIST_H
#define FAREBL_LIST_H



#include <cstddef>           // for size_t, ptrdiff_t
#include <initializer_list>  // for initializer_list
#include <iterator>          // for make_reverse_iterator, reverse_iterator
#include <limits>            // for numeric_limits
#include <memory>            // for allocator_traits, allocator
#include <type_traits>       // for conditional
#include <utility>           // for forward

namespace Farebl {

template<typename T, typename Allocator = std::allocator<T>>
class list{
    struct BaseNode{
        BaseNode* prev;
        BaseNode* next;
        BaseNode(BaseNode* prev, BaseNode* next): prev(prev), next(next){}
    };
    
    struct Node: BaseNode{
        T value;
    };

    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    
    NodeAllocator alloc_;
    BaseNode fake_node_;
    size_t sz_;

    template<bool IsConst = false>
    struct base_iterator{
    private:
        friend class Farebl::list<T, Allocator>;

        using BaseNodePtr_t = typename std::conditional<IsConst, const BaseNode*, BaseNode*>::type;
        using NodePtr_t = typename std::conditional<IsConst, const Node*, Node*>::type;
        
        BaseNodePtr_t ptr_;
        base_iterator(BaseNodePtr_t ptr):ptr_(ptr){}
    
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type	    = T;
        using pointer           = typename std::conditional<IsConst, const T*, T*>::type;
        using reference         = typename std::conditional<IsConst, const T&, T&>::type; 
        using iterator_category = std::bidirectional_iterator_tag;
            
        base_iterator(const base_iterator& other) = default;
        base_iterator& operator=(const base_iterator& other) = default;
        
        reference operator*()const {
            return static_cast<NodePtr_t>(ptr_)->value;
        }
        
        pointer operator->()const{
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

public:

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
    


    list() 
        : alloc_()
        , fake_node_(&fake_node_, &fake_node_)
        , sz_(0)
    {}

    explicit list(const Allocator& alloc)
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)  
        , sz_(0)
    {}
    
    explicit list(size_t count, const Allocator& alloc) 
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)
        , sz_(0)    
    {
        insert(begin(), T(), count);
    }
    
    explicit list(size_t count, const T& value, const Allocator& alloc = Allocator()) 
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)
        , sz_(0)    
    {
        insert(begin(), count, value);
    }

    
    template <typename InputIt>
    list(InputIt first, InputIt last, const Allocator& alloc = Allocator()) 
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)
        , sz_(0)    
    {
        insert(begin(), first, last);
    }

    list(const list& other) 
        : alloc_(std::allocator_traits<NodeAllocator>::select_on_container_copy_construction(other.get_allocator()))
        , fake_node_(&fake_node_, &fake_node_)  
        , sz_(0)
    {
        insert(begin(), other.begin(), other.end());        
    }
    

    list(list&& other) 
        : alloc_(std::move(other.get_allocator()))
        , fake_node_(&fake_node_, &fake_node_)  
        , sz_(other.sz_)
    {
        fake_node_.next = other.fake_node_.next;
        fake_node_.prev = other.fake_node_.prev;
         
        fake_node_.next->prev = &fake_node_;
        fake_node_.prev->next = &fake_node_;

        other.fake_node_.next = nullptr;
        other.fake_node_.prev = nullptr;
        other.sz_ = 0;
    }

    
    list(const list& other, const Allocator& alloc) 
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)  
        , sz_(0)
    {
        insert(begin(), other.begin(), other.end());        
    }
   

    list(list&& other, const Allocator& alloc) 
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)  
        , sz_(other.sz_)
    {
        fake_node_.next = other.fake_node_.next;
        fake_node_.prev = other.fake_node_.prev;
         
        fake_node_.next->prev = &fake_node_;
        fake_node_.prev->next = &fake_node_;

        other.fake_node_.next = &other.fake_node_;
        other.fake_node_.prev = &other.fake_node_;
        other.sz_ = 0;
    }



    list(std::initializer_list<T> init_list, const Allocator& alloc = Allocator())
        : alloc_(alloc)
        , fake_node_(&fake_node_, &fake_node_)
        , sz_(0)    
    {
        insert(begin(), init_list);
    }

    

    ~list() {
        clear();
    }



    list& operator=(const list& other) & {
        if (this == &other) return *this;
        
        NodeAllocator new_alloc = std::allocator_traits<NodeAllocator>::propagate_on_container_copy_assignment::value
           ?  NodeAllocator(other.get_allocator()) : alloc_;
        
        if constexpr (std::allocator_traits<NodeAllocator>::is_always_equal::value){
            alloc_ = new_alloc;
            assign(other.begin(), other.end());
        }
        else{
            if(alloc_ == new_alloc){
                alloc_ = new_alloc;
                assign(other.begin(), other.end());
            }
            else{
                //for strong exception safety using copy-and-swap idiom 
                if constexpr(std::allocator_traits<NodeAllocator>::propagate_on_container_swap::value){
                    list temp(other, new_alloc);
                    swap(temp);
                }
                else{
                    list temp(other, new_alloc);
                    swap(temp);
                    alloc_ = new_alloc;
                }
                /*
                    When implementing a basic exception guarantee (for best performance), a problem arises: 
                when propagate_on_copy_assignment::value == true and alloc_ != new_alloc 
                we have to allocate new elements with a new allocator, but in order to 
                preserve the old allocator (we need it to clean up old data), the operation 
                of replacing the current allocator with a new one (alloc_ = new_alloc) is 
                postponed to the end of the assignment operation.
                    But if an exception occurs during the element-by-element replacement of 
                elements, the current list ends up in a heterogeneous state: some elements 
                are created by the new allocator, and the rest by the old one. At the same 
                time, the current allocator has not yet been updated. This leads to the fact 
                that when cleaning up the current list, those elements that were deleted by 
                the new allocator (new_alloc) will try to be cleaned up by the old allocator 
                (alloc_). As mentioned above, alloc_ != new_alloc, so we expect a memory leak 
                or (UB).
                */
            }
        }
        return *this;
    }

    list& operator=(list&& other) & noexcept(noexcept(std::allocator_traits<NodeAllocator>::is_always_equal::value)) {
        if (this == &other) return *this;
        
        NodeAllocator new_alloc = std::allocator_traits<NodeAllocator>::propagate_on_container_move_assignment::value  
           ?  NodeAllocator(other.get_allocator()) : alloc_;
        
        if constexpr (std::allocator_traits<NodeAllocator>::is_always_equal::value){
            clear();
            alloc_ = new_alloc;
            fake_node_.next = other.fake_node_.next;
            fake_node_.prev = other.fake_node_.prev;
            fake_node_.next->prev = &fake_node_;
            fake_node_.prev->next = &fake_node_;
            sz_ = other.sz_;

            other.fake_node_.next = &other.fake_node_;
            other.fake_node_.prev = &other.fake_node_;
            other.sz_ = 0;
        }
        else{
            if(alloc_ == new_alloc){
                clear();
                alloc_ = new_alloc;
                fake_node_.next = other.fake_node_.next;
                fake_node_.prev = other.fake_node_.prev;
                fake_node_.next->prev = &fake_node_;
                fake_node_.prev->next = &fake_node_;

                sz_ = other.sz_;

                other.fake_node_.next = nullptr;
                other.fake_node_.prev = nullptr;
                other.sz_ = 0;
            }
            else{ 
                //for strong exception safety using copy-and-swap idiom 
                if constexpr(std::allocator_traits<NodeAllocator>::propagate_on_container_swap::value){
                    list temp(std::move(other), new_alloc);
                    swap(temp);
                }
                else{
                    list temp(std::move(other), new_alloc);
                    swap(temp);
                    alloc_ = new_alloc;
                }

                /*
                When implementing the basic exception guarantee (for the best performance),
                the same problem arises as with operator=(copy): if an exception occurs 
                during element-by-element replacement of elements, the current list enters 
                a heterogeneous state: some elements are created by the new allocator, and 
                the rest by the old one. At the same time, the current allocator has not 
                yet been updated. This leads to the fact that when cleaning up the current 
                list, those elements that were removed by the new allocator (new_alloc) 
                will try to be cleaned up by the old allocator (alloc_), while 
                alloc_ != new_alloc, so we expect a memory leak or (UB).
                */
            } 
        }
       return *this;
        
    }

    list& operator=(std::initializer_list<T> init_list) & {
        assign(init_list.begin(), init_list.end());
        return *this;
    }
    

    void assign(size_t count, const T& value){
        if (count == 0){
            clear();
            return;
        }
        
        BaseNode* current_node = fake_node_.next;
        if(count <= sz_){
            for (int i = 0; i<count; ++i){
                static_cast<Node*>(current_node)->value = value;
                current_node = current_node->next;
            } 
            erase(current_node, cend());
        }
        else{
            for (int i = 0; i<sz_; ++i){
                static_cast<Node*>(current_node)->value = value;
                current_node = current_node->next;
            }
            current_node = current_node->prev;
            insert(current_node, count-sz_, value);
        }
    }
    
    template< class InputIt>
    void assign(InputIt first, InputIt last){
        if (first == last){return;}
        
        InputIt current_other_it = first;
        BaseNode* current_node = fake_node_.next;
        while (current_node != &fake_node_ && current_other_it != last){
            static_cast<Node*>(current_node)->value = *current_other_it;

            current_node = current_node->next;
            ++current_other_it;
        } 
        if (current_node == &fake_node_){
            insert(current_node->prev, current_other_it, last);
        }
        else{
            erase(current_node, cend());
        }        
    }


    void assign(std::initializer_list<T> init_list){
        assign(init_list.begin(), init_list.end());
    }



    Allocator get_allocator() const {return alloc_;}



    reference front(){return *begin();}
    const_reference front() const {return *cbegin();}

    reference back(){return *(--end());}
    const_reference cback() const {return *(--cend());}
 
    

    iterator begin(){return {fake_node_.next};}
    iterator end(){return {&fake_node_};}
    
    const_iterator begin() const {return fake_node_.next;}
    const_iterator end() const {return {&fake_node_};}

    const_iterator cbegin() const noexcept {return {fake_node_.next};}
    const_iterator cend() const noexcept {return {&fake_node_};}


    /*
    The std::make_reverse_iterator function in C++, when creating a reverse iterator,
    automatically shifts the iterator so that it points to an element that matches the 
    logic of the reverse iterator.
    */
    reverse_iterator rbegin(){return std::make_reverse_iterator(begin());}
    reverse_iterator rend(){return std::make_reverse_iterator(end());}
    
    const_reverse_iterator rbegin() const {return std::make_reverse_iterator(cbegin());}
    const_reverse_iterator rend() const {return std::make_reverse_iterator(cend());}
    
    const_reverse_iterator crbegin() const noexcept {return std::make_reverse_iterator(cbegin());}
    const_reverse_iterator crend() const noexcept {return std::make_reverse_iterator(cend());}


    bool empty() const {return sz_ == 0;}
    
    size_t size() const {return sz_;}
    
    long max_size() const{return std::numeric_limits<difference_type>::max();}

 
    void clear(){ erase(cbegin(), cend());}
    

    iterator insert(const_iterator pos, const T& value){
        Node* new_node = nullptr; 
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, value);
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
       
        new_node->next = pos_node;
        new_node->prev = pos_node->prev;

        new_node->prev->next = new_node;
        pos_node->prev = new_node;

        ++sz_;
        return {new_node};
    }

    
    iterator insert(const_iterator pos, T&& value){
        Node* new_node = nullptr; 
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, std::move(value));
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
       
        new_node->next = pos_node;
        new_node->prev = pos_node->prev;

        new_node->prev->next = new_node;
        pos_node->prev = new_node;

        ++sz_;
        return {new_node};
    }


    iterator insert( const_iterator pos, size_type count, const T& value ){
        if (count == 0) return const_cast<BaseNode*>(pos.ptr_);

        BaseNode* first_new_node = nullptr;
        size_t inserted_count = 0;
        first_new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        
        try{
            std::allocator_traits<NodeAllocator>::construct(alloc_, &static_cast<Node*>(first_new_node)->value, value);
        }
        catch(...){
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(first_new_node), 1);
            throw;
        } 
        
       ++inserted_count;

        BaseNode* last_new_node = first_new_node;
        try{
            for(; inserted_count != count; ++inserted_count){
                last_new_node->next = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
                try{
                    /*The Node::prev and Node::next elements are of type (BaseNode*).*/
                    std::allocator_traits<NodeAllocator>::construct(alloc_, &static_cast<Node*>(last_new_node->next)->value, value);
                }
                catch(...){
                    std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(last_new_node->next), 1);
                    throw;
                }
                last_new_node->next->prev = last_new_node;
                last_new_node = last_new_node->next;
            }
        }
        catch(...){
            while(last_new_node != first_new_node){
                last_new_node = last_new_node->prev;
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(last_new_node->next));
                /*The Node::prev and Node::next elements are of type (BaseNode*).*/
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(last_new_node->next), 1);
                --inserted_count;
            }

            //Removing the second new node
            if(inserted_count == 2){
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(first_new_node->next));
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(first_new_node->next), 1); 
            }

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

        last_new_node->next = pos_node;
        first_new_node->prev = pos_node->prev;
        
        first_new_node->prev->next = first_new_node;
        pos_node->prev = last_new_node;

        sz_+=inserted_count;
        return {first_new_node};
    }   

    template< class InputIt > 
    iterator insert(const_iterator pos, InputIt first, InputIt last){
        if (first == last) return const_cast<BaseNode*>(pos.ptr_);

        auto current_other_it = first;
        BaseNode* first_new_node = nullptr;
        size_t inserted_count = 0;
        
        first_new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{
            std::allocator_traits<NodeAllocator>::construct(alloc_, &static_cast<Node*>(first_new_node)->value, *current_other_it);
        }
        catch(...){
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(first_new_node), 1);
            throw;
        } 
        
        ++inserted_count;
        ++current_other_it;

        BaseNode* last_new_node = first_new_node;
        try{
            while(current_other_it != last){
                last_new_node->next = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
                try{
                    /*The Node::prev and Node::next elements are of type (BaseNode*).*/
                    std::allocator_traits<NodeAllocator>::construct(alloc_, &static_cast<Node*>(last_new_node->next)->value, *current_other_it);
                }
                catch(...){
                    std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(last_new_node->next), 1);
                    throw;
                }
                
                last_new_node->next->prev = last_new_node;
                last_new_node = last_new_node->next;
                ++current_other_it;
                ++inserted_count;
            }
        }
        catch(...){
            while(last_new_node != first_new_node){
                last_new_node = last_new_node->prev;
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(last_new_node->next));
                /*The Node::prev and Node::next elements are of type (BaseNode*).*/
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(last_new_node->next), 1);
                --inserted_count;
            }

            //Removing the second new node
            if(inserted_count == 2){
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(first_new_node->next));
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(first_new_node->next), 1); 
            }

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

        last_new_node->next = pos_node;
        first_new_node->prev = pos_node->prev;
        
        pos_node->prev->next = first_new_node;
        pos_node->prev = last_new_node;

        sz_+=inserted_count;
        return {first_new_node};
    }      


    iterator insert( const_iterator pos, std::initializer_list<T> init_list ){
        return insert(pos, init_list.begin(), init_list.end());
    }      
 

    
    
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
        return {delete_node_next};
    }


    iterator erase(const_iterator first, const_iterator last){
        if(first == last) return const_cast<BaseNode*>(last.ptr_);
        
        /* 
        The BaseNode::next and BaseNode::prev fields are non-constant, 
        but the const_iterator::ptr_ pointer is const 
        */
        BaseNode* first_prev = first.ptr_->prev;  
        BaseNode* following_after_delete = first.ptr_->next;

        if(last != cend()){
             while (following_after_delete->prev != last.ptr_) { 
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(following_after_delete->prev));
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(following_after_delete->prev), 1);    
                --sz_;
                following_after_delete = following_after_delete->next;
            }

            first_prev->next = following_after_delete->prev;
            following_after_delete->prev->prev = first_prev;
        }
        else{ 
            while (following_after_delete != &fake_node_) { 
                std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(following_after_delete->prev));
                std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(following_after_delete->prev), 1);    
                --sz_;
                following_after_delete = following_after_delete->next;
            }
            
            std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(following_after_delete->prev));
            std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(following_after_delete->prev), 1);    
            --sz_;
            first_prev->next = following_after_delete;
            following_after_delete->prev = first_prev;
        }

        return {following_after_delete};
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
        Node* new_node;
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{ 
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, value);
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
    }
    
    void push_back(T&& value){
        Node* new_node;
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{ 
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, std::move(value));
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
    }



    template <class... Args>
    reference emplace_back(Args&&... args){
        Node* new_node;
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{ 
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, std::forward<Args>(args)...);
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

        return new_node->value;
    }


    void pop_back(){
        if (empty()) {return;}

        BaseNode* delete_node = fake_node_.prev;
        fake_node_.prev = delete_node->prev;
        fake_node_.prev->next = &fake_node_; 
        std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(delete_node));
        std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(delete_node), 1);
        --sz_;
    }
    


    void push_front(const T& value){
        Node* new_node;
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, value);
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
    }
    
    void push_front(T&& value){ 
        Node* new_node;
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, std::move(value));
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
    }


    template <class... Args>
    reference emplace_front(Args&&... args){
        Node* new_node;
        new_node = std::allocator_traits<NodeAllocator>::allocate(alloc_, 1);
        try{ 
            std::allocator_traits<NodeAllocator>::construct(alloc_, &new_node->value, std::forward<Args>(args)...);
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

        return new_node->value;
    }




    void pop_front(){
        if (empty()) {return;}

        BaseNode* delete_node = fake_node_.next;
        fake_node_.next = delete_node->next;
        fake_node_.next->prev = &fake_node_; 
        std::allocator_traits<NodeAllocator>::destroy(alloc_, static_cast<Node*>(delete_node));
        std::allocator_traits<NodeAllocator>::deallocate(alloc_, static_cast<Node*>(delete_node), 1);
        --sz_;
    }

   
    void resize(size_type count){
        if (count > sz_){
            insert(--end(), count - sz_, T());
        }
        else if (count < sz_){
            BaseNode* first_delete;
            if (count <= sz_/2){
                first_delete = fake_node_.next;
                for (int i = 0; i < count; ++i){
                    first_delete = first_delete->next;
                }
            }
            else{
                first_delete = &fake_node_;
                for (int i = sz_; i > count; --i){
                    first_delete = first_delete->prev;
                }
            }
            erase(first_delete, cend());
        }
    }
    
    void resize(size_type count, const value_type& value){   
        if (count > sz_){
            insert(--end(), count - sz_, value);
        }
        else if (count < sz_){
            BaseNode* first_delete;
            if (count <= sz_/2){
                first_delete = fake_node_.next;
                for (int i = 0; i < count; ++i){
                    first_delete = first_delete->next;
                }
            }
            else{
                first_delete = &fake_node_;
                for (int i = sz_; i > count; --i){
                    first_delete = first_delete->prev;
                }
            }
            erase(first_delete, cend());
        }
    }
   
    void swap(list& other) noexcept(noexcept(std::allocator_traits<NodeAllocator>::is_always_equal::value)){
        std::swap(fake_node_, other.fake_node_);
        std::swap(fake_node_.next->prev, other.fake_node_.next->prev);
        std::swap(fake_node_.prev->next, other.fake_node_.prev->next);
        std::swap(sz_, other.sz_);
        
        if constexpr(std::allocator_traits<NodeAllocator>::propagate_on_container_swap::value){
            std::swap(alloc_, other.alloc_);
        }
    } 
    
   
   
    //merge
    
    
    //splice
    
    
    size_type remove(const T& value){
        BaseNode* current_node = fake_node_.next;
        size_t count = 0;
        while(current_node != &fake_node_){
            if (static_cast<Node*>(current_node)->value == value){
                current_node = current_node->next;
                erase(current_node->prev);
                ++count;
                continue;
            }
            current_node = current_node->next;
        }
        return count; 
    }

    template< class UnaryPred >
    size_type remove_if( UnaryPred p ){
        BaseNode* current_node = fake_node_.next;
        size_t count = 0;
        while(current_node != &fake_node_){
            if (p(static_cast<Node*>(current_node)->value)){
                current_node = current_node->next;
                erase(current_node->prev);
                ++count;
                continue;
            }
            current_node = current_node->next;
        }
        return count; 
    }


    //unique

    //sort
    
};


template <typename T, typename Allocator>
bool operator==(const list<T, Allocator>& lhs, const list<T, Allocator>& rhs){
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}


template <typename T, typename Allocator>
bool operator!=(const list<T, Allocator>& lhs, const list<T, Allocator>& rhs){
    return !(lhs == rhs);
}


template <typename T, typename Allocator>
bool operator<(const list<T, Allocator>& lhs, const list<T, Allocator>& rhs){
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}


template <typename T, typename Allocator>
bool operator>(const list<T, Allocator>& lhs, const list<T, Allocator>& rhs){
   
    return (rhs < lhs);
    /*
    
    self implementation of lexicographical_compare

    if (lhs.size() == 0) return false;
    if (rhs.size() == 0) return true;

    auto lhs_it = lhs.cbegin();
    auto lhs_end = lhs.cend();
    auto rhs_it = rhs.cbegin();
    auto rhs_end = rhs.cend();

    if (lhs.size() >= rhs.size()) {
        while(rhs_it != rhs_end){
            if (*lhs_it != *rhs_it){
                if (!(*lhs_it < *rhs_it)) return true;
                if (*lhs_it < *rhs_it) return false;
            }
            ++lhs_it;
            ++rhs_it;
        }
        if (lhs_it != lhs_end) 
            return true;
        else 
            return false;
    }
    else {
        while(lhs_it != lhs_end){
            if (*lhs_it != *rhs_it){
                if (!(*lhs_it < *rhs_it)) return true;
                if (*lhs_it < *rhs_it) return false;
            }
            ++lhs_it;
            ++rhs_it;
        }
        return false;
    }
    */
}


template <typename T, typename Allocator>
bool operator<=(const list<T, Allocator>& lhs, const list<T, Allocator>& rhs){
    return !(lhs > rhs); 
}


template <typename T, typename Allocator>
bool operator>=(const list<T, Allocator>& lhs, const list<T, Allocator>& rhs){
    return !(lhs < rhs);
}


//CTAD deduction guides

template <typename InputIterator, typename Allocator = std::allocator<typename std::iterator_traits<InputIterator>::value_type>>
list(InputIterator, InputIterator, Allocator = Allocator()) -> list<typename std::iterator_traits<InputIterator>::value_type, Allocator>; 


}// end namespace Farebl

#endif //FAREBL_LIST_H
