#include <iostream>
#include <iterator>
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
    typename Alloc::template rebind<Node>::other alloc_;
    size_t sz_;
    
public:

    template<bool IsConst = false>
    struct base_iterator{
    private:
        using pointer_type_   = std::conditional<IsConst, const T*, T*>;
        using refernece_type_ = std::conditional<IsConst, const T&, T&>;
        
        std::conditional<IsConst, const BaseNode*, BaseNode*> ptr_;
        
        
        base_iterator(pointer_type_ ptr):ptr_(ptr){}
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
        };
        pointer_type_ operator->(){
            return reinterpret_cast<Node*>(ptr_);
        };
        
        base_iterator& operator++(){return 
            ptr_ = ptr_->next;
            return *this;
        };
        base_iterator& operator++(int ){ 
            base_iterator<IsConst> temp = *this;
            ptr_ = ptr_->next;
            return temp;
        };
        base_iterator& operator--(){return 
            ptr_ = ptr_->prev;
            return *this;
        };
        base_iterator& operator--(int ){ 
            base_iterator<IsConst> temp = *this;
            ptr_ = ptr_->prev;
            return temp;
        };
        operator base_iterator<true>()const{return {ptr_};}
    };

    using value_type	  = T;
    using allocator_type  = Alloc;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference	      = value_type&;
    using const_reference = const value_type&;
    using pointer         = std::allocator_traits<Alloc>::pointer;      	 
    using const_pointer   = std::allocator_traits<Alloc>::const_pointer;

    using iterator               = base_iterator<false>;	
    using const_iterator         = base_iterator<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List() : fake_node_(&fake_node_, &fake_node_), alloc_(), sz_(0){}
   
    bool empty(){return fake_node_.next == fake_node_.prev;}
    void push_back(const T& value){
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
        
        if (empty()){
            new_node_ptr->next = &fake_node_;
            new_node_ptr->prev = &fake_node_;
            fake_node_->next = new_node_ptr;
            fake_node_->prev = new_node_ptr;
        }
        else{
            Node* fake_node_prev = fake_node_->prev;  
            new_node_ptr->prev = fake_node_prev;
            new_node_ptr->next = &fake_node_;           
            fake_node_->prev = new_node_ptr;
        }
        ++sz_;
    }

    void push_front(const T& value){
        if (empty()){
            push_back(value);
        }
        else{
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
            new_node_ptr->next = fake_node_next;           
            new_node_ptr->prev = &fake_node_;
            fake_node_->next = new_node_ptr;
        }
        ++sz_;
    }
    
};

int main(){
    return 0;
}
