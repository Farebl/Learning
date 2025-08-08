#ifndef FAREBL_DEQUE_H
#define FAREBL_DEQUE_H

#include <memory>

namespace Farebl{

template <typename T, typename Allocator = std::allocator<T>>
class deque{
private:
    T** buckets_; 
    T* first_;
    T* last_;
    size_t size_;
    Allocator alloc_;
    static const size_t bucket_size_ = (sizeof(T) < 256) ? 4096/sizeof(T) : 16; 

public:
    template <bool IsConst = false>
    class base_iterator{
    private:
        T** bucket_ptr_;
        T* ptr_;
        base_iterator(T** bucket_ptr, T* ptr): bucket_ptr_(bucket_ptr), ptr_(ptr){}
    
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = typename std::conditional<IsConst, const T*, T*>::type;
        using reference         = typename std::conditional<IsConst, const T&, T&>::type; 
        using iterator_category = std::random_access_iterator_tag;
    
        reference operator*() const {
            return *ptr_; 
        }

        pointer operator->() const {
            return ptr_;
        }


        base_iterator& operator++(){
            if (ptr_ - *bucket_ptr_ == deque<T, Allocator>::bucket_size_){
                ++bucket_ptr_;
                ptr_ = *bucket_ptr_;
            }
            return *this;
        }

        base_iterator operator++(int){
            base_iterator temp = *this; 
            ++(*this);
            return temp; 
        }


        base_iterator& operator--(){
            if (ptr_ - *bucket_ptr_ == 0){
                --bucket_ptr_;
                ptr_ = *bucket_ptr_;
            }
            return *this;
        }

        base_iterator operator--(int){
            base_iterator temp = *this; 
            --(*this);
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

        operator base_iterator<true>(){return {ptr_};}
    };


    //explicit deque(): fake_node_{nullptr, nullptr}, buckets_(nullptr), size_(0), alloc_(Allocator()){}

    
    //explicit deque(const Allocator& alloc){}
    
    //explicit deque(size_type count, const Allocator& alloc){}

    //deque(size_type count, const T& value, const Allocator& alloc){}

    /*
    template <typename InputIt>
    deque(InputIt first, InputIt last, const Allocator& alloc = Allocator()){}
    */

    //deque (const deque& other){}
    
    //deque (deque&& other){}
    
    //deque (const deque& other, const Allocator& alloc){}
    
    //deque (deque&& other, const Allocator& alloc){}

    //deque (std::initializer_list<T> init_list, const Allocator& alloc){}
    
    //~deque(){}
    

    //deque& operator=(const deque& other){}
    
    //deque& operator=(deque&& other)noexcept(noexcept(std::allocator_traits<Allocator>::is_always_equal::value)){}
    
    
    //deque& operator=(std::initializer_list<value_type> init_list){}
    
    // void assign(size_type count, const T& value);

    /*
    template <typename InputIt>
    void assign(InputIt first, InputIt last){} 
    */

    //void assign(std::initializer_list<T> init_list){}

    // allocator_type get_allocator() const {}

    // reference at(size_type pos){}
    // const_reference at(size_type pos) const {}

     
    // reference operator[](size_type pos){}
    // const_reference operator[](size_type pos) const {}

    
    // reference front(){}
    // const_reference front() const {}


    // reference back(){}
    // const_reference back() const {}


    // iterator begin(){}
    // const_iterator begin() const {}
    // const_iterator cbegin() const noexcept {}


    // iterator end(){}
    // const_iterator end() const {}
    // const_iterator cend() const noexcept{}


    // reverse_iterator rbegin(){}
    // const_reverse_iterator rbegin() const {}
    // const_reverse_iterator crbegin() const noexcept{}
    

    // reverse_iterator rend(){}
    // const_reverse_iterator rend() const {}
    // const_reverse_iterator crend() const noexcept {}


    // bool empty() const {}
    
    // size_type size() const {}
    
    // size_type max_size() const {}
    
    // void shrink_to_fit() {}


    // void clear() {}
    

    // iterator insert(const_iterator pos, const T& value){}
    
    // iterator insert(const_iterator pos, T&& value){}
    
    // iterator insert(const_iterator pos, size_type count, const T& value){}
    
    /*
    template <typename InputIt> 
    iterator insert(const_iterator pos, InputIt first, InputIt last){}
    */
    
    //iterator insert(std::initializer_list<T> init_list){}

};


} // end namespace Farebl
#endif // FAREBL_DEQUE_H
