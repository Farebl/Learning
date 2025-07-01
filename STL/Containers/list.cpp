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
        Node (T value, BaseNode* prev = nullptr, BaseNode* next = nullptr): BaseNode(prev, next), value(value){}
    };
    BaseNode fake_node_;
    typename Alloc::template rebind<Node>::other alloc_;
    size_t sz_;
    
public:

    template<bool IsConst = false>
    struct base_iterator{
    private:
        using type_           = std::conditional<IsConst, const BaseNode, BaseNode>; 
        using pointer_type_   = std::conditional<IsConst, BaseNode*, const BaseNode*>;
        using refernece_type_ = std::conditional<IsConst, BaseNode&, const BaseNode&>;
        
        pointer_type_ ptr_;
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type	    = type_;
        using pointer           = pointer_type_;
        using reference         = refernece_type_;
        using iterator_category = std::bidirectional_iterator_tag;
        
        base_iterator(pointer_type_ ptr):ptr_(ptr){}
        
        base_iterator(const base_iterator& other) = default;
        base_iterator& operator=(const base_iterator& other) = default;
        
        refernece_type_ operator*(){return *ptr_;};
        pointer_type_ operator->(){return ptr_;};
        
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

    List() : fake_node_(&fake_node_, &fake_node_), alloc_(), sz_(0){}
    
    //List(size_t n) : fake_node_(&fake_node_, &fake_node_), alloc_(), sz_(n){}
    


};

int main(){
    return 0;
}
