#ifndef FAREBL_DEQUE_H
#define FAREBL_DEQUE_H

#include <algorithm>
#include <memory>
#include <limits>


namespace Farebl{

template <typename T, typename Alloc = std::allocator<T>, size_t BucketSize = ((sizeof(T) < 256) ? 4096/sizeof(T) : 16)>
class deque{

    static_assert(BucketSize > 0, "The bucket size must be 1 or greater");
    static_assert(BucketSize <= 104'857'600/sizeof(T), "The bucket size cannot exceed 100 MB");
        
private:
    template <bool IsConst = false>
    class base_iterator{

    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = typename std::conditional<IsConst, const T*, T*>::type;
        using reference         = typename std::conditional<IsConst, const T&, T&>::type; 
        using iterator_category = std::random_access_iterator_tag;

    private:
        friend class deque;
        T** m_buckets_ptr;
        size_t m_buckets_capacity;
        difference_type m_pseudo_cell_index; 
    /*      
        m_pseudo_cell_index: 
        if (m_pseudo_cell_index == -1) --> it means that it is valid iterator
        else --> it means index of (pseudo-cell), in pseudo bucket
    */
        T** m_bucket_ptr;
        pointer m_ptr;
        base_iterator():
            m_buckets_ptr(nullptr), 
            m_buckets_capacity(0),
            m_pseudo_cell_index(-1),
            m_bucket_ptr(nullptr), 
            m_ptr(nullptr){}

        base_iterator(T** buckets_ptr, size_t buckets_capacity, T** bucket_ptr, pointer ptr):  
            m_buckets_ptr(buckets_ptr), 
            m_buckets_capacity(buckets_capacity),
            m_pseudo_cell_index(-1),
            m_bucket_ptr(bucket_ptr), 
            m_ptr(ptr){}
    public:

        reference operator*() const {return *m_ptr; }

	    pointer operator->() const {return m_ptr;}

        base_iterator& operator++(){
            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    if(((const_cast<T*>(m_ptr) - *m_bucket_ptr) + 1) == static_cast<difference_type>(BucketSize)){
                        ++m_bucket_ptr;
                        if (
                            (m_bucket_ptr >= m_buckets_ptr)
                                &&
                            (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                        ){
                            m_ptr = *m_bucket_ptr;
                        }
                        else{
                            m_ptr = nullptr;
                            m_pseudo_cell_index = 0; // iterator points of 1-st pseudo cell of pseudo bucket
                        }  
                    }
                    else{
                        ++m_ptr;
                    }
                }
                else {
                    if(m_pseudo_cell_index == (static_cast<difference_type>(BucketSize) - 1)){
                        ++m_bucket_ptr;
                        if (
                            (m_bucket_ptr >= m_buckets_ptr)
                                &&
                            (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                        ){
                            m_ptr = *m_bucket_ptr;
                            m_pseudo_cell_index = -1;
                        }
                        else{
                            m_pseudo_cell_index = 0;
                        }
                    }
                    else{
                        ++m_pseudo_cell_index;
                    }
                }
            }            
            return *this;
        }

        base_iterator operator++(int){
            base_iterator temp = *this; 
            ++(*this);
            return temp; 
        }


        base_iterator& operator--(){
            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    if (m_ptr == *m_bucket_ptr){
                        --m_bucket_ptr;
                        if (
                            (m_bucket_ptr >= m_buckets_ptr)
                                &&
                            (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                        ){
                            m_ptr = *m_bucket_ptr + static_cast<difference_type>(BucketSize) - 1;
                        }
                        else{
                            m_ptr = nullptr;
                            m_pseudo_cell_index = static_cast<difference_type>(BucketSize) - 1; // iterator points of last pseudo cell of pseudo bucket
                        }
                    }   
                    else{
                        --m_ptr;
                    }
                }
                else{
                    if(m_pseudo_cell_index == 0){
                        --m_bucket_ptr;
                        if (
                            (m_bucket_ptr >= m_buckets_ptr)
                                &&
                            (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                        ){
                            m_ptr = *m_bucket_ptr + static_cast<difference_type>(BucketSize) - 1;
                            m_pseudo_cell_index = -1;
                        }
                        else{
                            m_pseudo_cell_index = static_cast<difference_type>(BucketSize) - 1;
                        }
                    }
                    else{
                        --m_pseudo_cell_index;
                    }
                }
            }
            return *this;
        }

        base_iterator operator--(int){
            base_iterator temp = *this; 
            --(*this);
            return temp; 
        }
        
        
        base_iterator& operator+=(difference_type value) & {
            if (value < 0) return *this -= value;
            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    difference_type result_index = (const_cast<T*>(m_ptr) - *m_bucket_ptr) + (value % BucketSize);
                    
                    if (value >= static_cast<difference_type>(BucketSize)){
                        m_bucket_ptr += value / BucketSize;
                    }
                    if (
                        (m_bucket_ptr >= m_buckets_ptr)
                            &&
                        (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                    ){ 
                        if (result_index < static_cast<difference_type>(BucketSize)){
                            m_ptr = *m_bucket_ptr + result_index;
                        }
                        else{
                            ++m_bucket_ptr;
                            if (
                                (m_bucket_ptr >= m_buckets_ptr)
                                    &&
                                (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                            ){ 
                                m_ptr = *m_bucket_ptr + (result_index - BucketSize);
                            }
                            else{
                                m_ptr = nullptr;
                                m_pseudo_cell_index = result_index - static_cast<difference_type>(BucketSize); 
                            }
                        }
                    }
                    else{
                        m_ptr = nullptr;
                        if (result_index < static_cast<difference_type>(BucketSize)){
                            m_pseudo_cell_index = result_index;
                        }
                        else{
                            ++m_bucket_ptr;
                            // there isn`t necessary to check boundaries: we have already gone beyond them
                            m_pseudo_cell_index = result_index - static_cast<difference_type>(BucketSize);
                        }
                    }
                }
                else{
                    difference_type result_pseudo_index = m_pseudo_cell_index + (value % BucketSize);
                    if (value >= static_cast<difference_type>(BucketSize)){
                        m_bucket_ptr += value / BucketSize;
                    }
                    if (
                        (m_bucket_ptr >= m_buckets_ptr)
                            &&
                        (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                    ){
                        if (result_pseudo_index < static_cast<difference_type>(BucketSize)){
                            difference_type result_index = m_pseudo_cell_index + (value % BucketSize);
                            m_ptr = *m_bucket_ptr + result_index;
                            m_pseudo_cell_index = -1;
                        }
                        else{
                            ++m_bucket_ptr;
                            if (
                                (m_bucket_ptr >= m_buckets_ptr)
                                    &&
                                (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                            ){ 
                                 m_ptr = *m_bucket_ptr + (result_pseudo_index - static_cast<difference_type>(BucketSize));
                                 m_pseudo_cell_index = -1;
                            }
                            else{
                                 m_pseudo_cell_index = result_pseudo_index - static_cast<difference_type>(BucketSize);
                            }
                        }
                    }
                    else{
                        if (result_pseudo_index < static_cast<difference_type>(BucketSize)){
                            m_pseudo_cell_index = result_pseudo_index;
                        }
                        else{
                            ++m_bucket_ptr;
                            if (
                                (m_bucket_ptr >= m_buckets_ptr)
                                    &&
                                (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                            ){
                                m_ptr = *m_bucket_ptr + (result_pseudo_index - static_cast<difference_type>(BucketSize));
                                m_pseudo_cell_index = -1;
                            }
                            else{
                                m_pseudo_cell_index = result_pseudo_index - static_cast<difference_type>(BucketSize);
                            }
                        }
                    }
                }
            }
            return *this;
        }

        base_iterator& operator-=(difference_type value) & {
            if (value < 0) {return *this += value;}
            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    difference_type result_index_in_bucket = (const_cast<T*>(m_ptr) - *m_bucket_ptr) - (value % BucketSize);
                    
                    if (value > static_cast<difference_type>(BucketSize)){
                        m_bucket_ptr -= value / BucketSize;
                    }
                    if (
                        (m_bucket_ptr >= m_buckets_ptr)
                            &&
                        (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                    ){ 
                        if (result_index_in_bucket > -1){
                            m_ptr = *m_bucket_ptr + result_index_in_bucket;
                        } 
                        else{
                            --m_bucket_ptr;
                            if (
                                (m_bucket_ptr >= m_buckets_ptr)
                                    &&
                                (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                            ){
                                m_ptr = *m_bucket_ptr + (BucketSize + result_index_in_bucket);
                            }
                            else{
                                m_ptr = nullptr;
                                m_pseudo_cell_index = BucketSize + result_index_in_bucket;
                            }
                        }
                    }
                    else{
                        m_ptr = nullptr;
                        if (result_index_in_bucket > -1){
                            m_pseudo_cell_index = result_index_in_bucket;
                        } 
                        else{
                            --m_bucket_ptr;
                            m_pseudo_cell_index = BucketSize + result_index_in_bucket;
                        }
                    }
                }
                else {
                    difference_type result_pseudo_index_in_bucket = m_pseudo_cell_index - (value % BucketSize);
                    if (value > static_cast<difference_type>(BucketSize)){
                        m_bucket_ptr -= value / BucketSize;
                    }
                    if (
                        (m_bucket_ptr >= m_buckets_ptr)
                            &&
                        (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                    ){
                        if (result_pseudo_index_in_bucket > -1){
                            m_ptr = *m_bucket_ptr + result_pseudo_index_in_bucket;
                            m_pseudo_cell_index = -1;
                        } 
                        else{
                            --m_bucket_ptr;
                            if (
                                (m_bucket_ptr >= m_buckets_ptr)
                                    &&
                                (m_bucket_ptr < (m_buckets_ptr + m_buckets_capacity))
                            ){
                                m_ptr = *m_bucket_ptr + (BucketSize + result_pseudo_index_in_bucket);
                                m_pseudo_cell_index = -1;
                            }
                            else{
                                m_pseudo_cell_index = (BucketSize + result_pseudo_index_in_bucket);
                            }
                        }
                    }
                    else{
                        if (result_pseudo_index_in_bucket > -1){
                             m_pseudo_cell_index = result_pseudo_index_in_bucket;
                        } 
                        else{
                            --m_bucket_ptr;
                            m_pseudo_cell_index = (BucketSize + result_pseudo_index_in_bucket);
                        }
                    }
                }
            }
            return *this;
        }


        base_iterator operator+(difference_type value) const {
            base_iterator temp = *(this);
            if (value<0)
                temp-=value;
            else
                temp+=value;

            return temp;
        }
        template <bool OtherIsConst>
        friend base_iterator operator+(difference_type value, const base_iterator<OtherIsConst>& it) {
            base_iterator temp = it;
            if (value<0)
                temp-=value;
            else
                temp+=value;

            return temp; 
        }


        base_iterator operator-(difference_type value){
            base_iterator temp = *this; 
            temp -= value;
            return temp; 
        }


        template<bool OtherIsConst>
        difference_type operator-(const base_iterator<OtherIsConst>& other){
            if (m_bucket_ptr == nullptr || other.m_bucket_ptr == nullptr){
                return m_bucket_ptr - other.m_bucket_ptr;
            }

            if(m_bucket_ptr == other.m_bucket_ptr){
                return const_cast<T*>(m_ptr) - const_cast<T*>(other.m_ptr); 
            }
            else if(m_bucket_ptr > other.m_bucket_ptr){
                return (
                    (((m_bucket_ptr - other.m_bucket_ptr) -1) * BucketSize) 
                    + 
                    (const_cast<T*>(m_ptr) - *m_bucket_ptr) + ((*other.m_bucket_ptr + BucketSize) - const_cast<T*>(other.m_ptr))
                );
            }
            else{
                return( 
                    (((other.m_bucket_ptr - m_bucket_ptr) -1) * BucketSize)
                    + 
                    (const_cast<T*>(other.m_ptr) - *other.m_bucket_ptr) + ((*m_bucket_ptr + BucketSize) - const_cast<T*>(m_ptr))
                ); 
            }
        }


        base_iterator& operator[](size_t index){return *(*this+index);}


        template<bool OtherIsConst>
        bool operator==(const base_iterator<OtherIsConst>& other){
            return (
                m_buckets_ptr == other.m_buckets_ptr 
                    && 
                m_bucket_ptr == other.m_bucket_ptr 
                    && 
                m_ptr == other.m_ptr); 
        }
        template<bool OtherIsConst>
        bool operator!=(const base_iterator<OtherIsConst>& other){
            return !(m_ptr == other.m_ptr);
        }

        template<bool OtherIsConst>
        bool operator<(const base_iterator<OtherIsConst>& other){
            return m_bucket_ptr < other.m_bucket_ptr ? true : (m_bucket_ptr == other.m_bucket_ptr && m_ptr < other.m_ptr) ? true : false; 
        }
        
        template<bool OtherIsConst>
        bool operator>(const base_iterator<OtherIsConst>& other){return other < *this;}

        template<bool OtherIsConst>
        bool operator>=(const base_iterator<OtherIsConst>& other){    return !(*this < other); }

        template<bool OtherIsConst>
        bool operator<=(const base_iterator<OtherIsConst>& other){    return !(*this > other); }

        operator base_iterator<true>(){return {m_buckets_ptr, m_buckets_capacity, m_bucket_ptr, const_cast<const T*>(m_ptr)};}
    };
public: 
    using value_type             = T;
    using allocator_type         = typename std::allocator_traits<Alloc>::template rebind_alloc<T>;
    using pointer                = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer          = typename std::allocator_traits<allocator_type>::const_pointer;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using R_val_reference        = value_type&&;
    using const_R_val_reference  = const value_type&&;
    using size_type              = size_t;
    using difference_type        = typename base_iterator<false>::difference_type;   

    using iterator               = base_iterator<false>;
    using const_iterator         = base_iterator<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
    T** m_buckets_ptr; 
    T** m_first_allocated_bucket_ptr;
    T** m_last_allocated_bucket_ptr;
    base_iterator<false> m_first;
    base_iterator<false> m_last;
    size_type m_size;
    size_type m_buckets_capacity;
    using Allocator = allocator_type;
    Allocator m_alloc;
    using AllocatorPtrOnBucket = typename std::allocator_traits<Allocator>::template rebind_alloc<T*>;
    AllocatorPtrOnBucket m_alloc_ptr_on_bucket;


    void center_the_iterators_m_first_and_m_last_(){
    /*
        Moving iterators (m_first and m_last) to the begin of the middle allocated bucket of the deque,
        to optimize subsequent operations of inserting elements in the begin or end.
    */
        size_t index_of_middle_allocated_bucket = 
            (m_first_allocated_bucket_ptr - m_buckets_ptr)
                +
            ((m_last_allocated_bucket_ptr - m_first_allocated_bucket_ptr) / 2);

        m_last.m_bucket_ptr = m_buckets_ptr + index_of_middle_allocated_bucket;
        m_last.m_ptr = *m_last.m_bucket_ptr;
        m_first = m_last; 
    }

    struct NewPtrsAndCapAfterRealloc{
        T** new_m_buckets_ptr; 
        T** new_m_first_allocated_bucket_ptr;
        T** new_m_last_allocated_bucket_ptr;
        iterator new_m_first;
        iterator new_m_last;
        size_t new_m_buckets_capacity;
    };

    NewPtrsAndCapAfterRealloc realloc_with_add_allocated_buckets_to_end(size_t count_of_buckets){
        NewPtrsAndCapAfterRealloc result;
        if(!m_buckets_ptr){
            result.new_m_buckets_capacity = count_of_buckets;
            result.new_m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, result.new_m_buckets_capacity);
            
            result.new_m_first_allocated_bucket_ptr = result.new_m_buckets_ptr;
            result.new_m_last_allocated_bucket_ptr = result.new_m_first_allocated_bucket_ptr;
            
            try{ //strong exception safety
                for (size_t successful_allocated_buckets = 0; successful_allocated_buckets < count_of_buckets; ++successful_allocated_buckets, ++result.new_m_last_allocated_bucket_ptr){
                    *result.new_m_last_allocated_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                }
                --result.new_m_last_allocated_bucket_ptr;
            }
            catch(...){
                --result.new_m_last_allocated_bucket_ptr;
                T** end_pos = result.new_m_buckets_ptr - 1;
                while(result.new_m_last_allocated_bucket_ptr != end_pos){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *result.new_m_last_allocated_bucket_ptr, BucketSize);
                    --result.new_m_last_allocated_bucket_ptr;
                }
                throw;
            }
            result.new_m_first.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_first.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_first.m_bucket_ptr = result.new_m_first_allocated_bucket_ptr;
            result.new_m_first.m_ptr = *result.new_m_first.m_bucket_ptr;

            result.new_m_last = result.new_m_first;
        }
        else{
            size_t capacity_from_begin_to_first_allocated_bucket = m_first.m_bucket_ptr - m_buckets_ptr;
            size_t count_of_used_buckets = m_last.m_bucket_ptr - m_first.m_bucket_ptr + 1;
            result.new_m_buckets_capacity = capacity_from_begin_to_first_allocated_bucket + (count_of_used_buckets * 2) + count_of_buckets;
            
            result.new_m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, result.new_m_buckets_capacity);
            
            result.new_m_last_allocated_bucket_ptr = result.new_m_buckets_ptr + capacity_from_begin_to_first_allocated_bucket + count_of_used_buckets; 
            size_t successful_allocated_buckets = 0;
            try{ //strong exception safety
                for (; successful_allocated_buckets < count_of_buckets; ++successful_allocated_buckets, ++result.new_m_last_allocated_bucket_ptr){
                    *result.new_m_last_allocated_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                }
                --result.new_m_last_allocated_bucket_ptr;
            }
            catch(...){
                --result.new_m_last_allocated_bucket_ptr;
                while(successful_allocated_buckets > 0){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *result.new_m_last_allocated_bucket_ptr, BucketSize);
                    --result.new_m_last_allocated_bucket_ptr;
                    --successful_allocated_buckets;
                }
                throw;
            }

            T** old_buckets_pos = m_last_allocated_bucket_ptr;
            result.new_m_first_allocated_bucket_ptr = result.new_m_last_allocated_bucket_ptr - count_of_buckets; 
            for (T** end_pos = m_first_allocated_bucket_ptr - 1; old_buckets_pos != end_pos; --old_buckets_pos, --result.new_m_first_allocated_bucket_ptr){
                *result.new_m_first_allocated_bucket_ptr = *old_buckets_pos;
            }
            ++result.new_m_first_allocated_bucket_ptr;
                       
            result.new_m_first.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_first.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_first.m_bucket_ptr = result.new_m_first_allocated_bucket_ptr + (m_first.m_bucket_ptr - m_first_allocated_bucket_ptr);
            result.new_m_first.m_ptr = m_first.m_ptr;       

            result.new_m_last.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_last.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_last.m_bucket_ptr = result.new_m_last_allocated_bucket_ptr - count_of_buckets - (m_last_allocated_bucket_ptr - m_last.m_bucket_ptr);
            result.new_m_last.m_ptr = m_last.m_ptr;
        }
        return result;
    }

public:

    explicit deque(): 
        m_buckets_ptr(nullptr), 
        m_first_allocated_bucket_ptr(nullptr),
        m_last_allocated_bucket_ptr(nullptr),
        m_first(m_buckets_ptr, 0, nullptr, nullptr), 
        m_last(m_buckets_ptr, 0, nullptr, nullptr),
        m_size(0), 
        m_buckets_capacity(0), 
        m_alloc(Allocator()),
        m_alloc_ptr_on_bucket(m_alloc)
    {}

    explicit deque(const Allocator& alloc): 
        m_buckets_ptr(nullptr), 
        m_first_allocated_bucket_ptr(nullptr),
        m_last_allocated_bucket_ptr(nullptr),
        m_first(m_buckets_ptr, 0, nullptr, nullptr), 
        m_last(m_buckets_ptr, 0, nullptr, nullptr), 
        m_size(0), 
        m_buckets_capacity(0), 
        m_alloc(alloc),
        m_alloc_ptr_on_bucket(m_alloc)
    {}
    
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
    
    ~deque(){
        erase(cbegin(), cend());
        shrink_to_fit();
    }
    

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


    reference operator[](size_type pos) & {
        return *((m_first + pos).m_ptr);
    }   
    const_reference operator[](size_type pos) const& {
        return *((m_first + pos).m_ptr);
    }

    R_val_reference operator[](size_type pos) && {
        return std::move(*((m_first + pos).m_ptr));
    }   
    const_R_val_reference operator[](size_type pos) const&& {
        return std::move(*((m_first + pos).m_ptr));
    }
    
    reference front() {return *m_first;}
    const_reference front() const {return *m_first;}

    reference back() {return *m_last;}
    const_reference back() const {return *m_last;}



    iterator begin() {return {m_first};}
    const_iterator begin() const {return {m_buckets_ptr, m_buckets_capacity, m_first.m_bucket_ptr, const_cast<const T*>(m_first.m_ptr)};}
    const_iterator cbegin() const noexcept {return begin();}

    //m_last pointing on the last element (not to the next position after last element, but straight at last element)
    iterator end() {
        if (m_buckets_ptr == nullptr){
            return {m_buckets_ptr, m_buckets_capacity, nullptr, nullptr};
        }
        return {m_last + 1 };
    }
    const_iterator end() const {
        if (m_buckets_ptr == nullptr){
            return {m_buckets_ptr, m_buckets_capacity, nullptr, nullptr};
        }
        return {m_last + 1};
    } 
    const_iterator cend() const noexcept {return end();}


    reverse_iterator rbegin() {return std::make_reverse_iterator<iterator>(end());}
    const_reverse_iterator rbegin() const {return std::make_reverse_iterator<const_iterator>(cend());}
    const_reverse_iterator crbegin() const noexcept {return rbegin();}
    

    reverse_iterator rend() {return std::make_reverse_iterator<iterator>(begin());}
    const_reverse_iterator rend() const {return std::make_reverse_iterator<const_iterator>(cbegin());}
    const_reverse_iterator crend() const noexcept {return rend();}



    bool empty() const {return !m_size;}
    
    size_type size() const {return m_size;}
    
    long max_size() const {return std::numeric_limits<difference_type>::max();}

    void shrink_to_fit(){
        if (m_buckets_ptr == nullptr){ return; }
         
        if (m_size == 0){
            T** end_pos = m_last_allocated_bucket_ptr + 1;
            while(m_first_allocated_bucket_ptr != end_pos){ 
                std::allocator_traits<Allocator>::deallocate(m_alloc, *m_first_allocated_bucket_ptr, BucketSize);
                ++m_first_allocated_bucket_ptr;
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, m_buckets_capacity);
            m_buckets_ptr = nullptr;
            m_first_allocated_bucket_ptr = nullptr; 
            m_last_allocated_bucket_ptr = nullptr;

            m_last.m_buckets_ptr = nullptr;
            m_last.m_buckets_capacity = 0;
            m_last.m_bucket_ptr = nullptr;
            m_last.m_ptr = nullptr;

            m_first = m_last;

            m_buckets_capacity = 0;
            return;
        }
       
        //deque is not empty:
        
        difference_type new_buckets_capacity = 
            (m_last.m_bucket_ptr - m_first.m_bucket_ptr)
            +
            (((m_last.m_ptr - *m_last.m_bucket_ptr) < (m_first.m_ptr - *m_first.m_bucket_ptr)) ? 0 : 1);
        
        T** new_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, new_buckets_capacity);
        decltype(new_buckets_capacity) success_allocated_count = 0;
        try{ //strong exception safety
            for (; success_allocated_count < new_buckets_capacity; ++success_allocated_count){
                new_buckets_ptr[success_allocated_count] = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
            }
        }
        catch(...){  
            for(decltype(success_allocated_count) i = 0; i < success_allocated_count; ++i){
                std::allocator_traits<Allocator>::deallocate(m_alloc, new_buckets_ptr[i], BucketSize);
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, new_buckets_ptr, new_buckets_capacity);
            throw;
        }

        iterator current_deque_it = m_first;
        iterator new_deque_it(new_buckets_ptr, new_buckets_capacity, new_buckets_ptr, *new_buckets_ptr);
        iterator end_pos = end();
        try{ //strong exception safety
            while(current_deque_it != end_pos){
                std::allocator_traits<Allocator>::construct(m_alloc, new_deque_it.m_ptr, std::move_if_noexcept(*current_deque_it));
                ++current_deque_it;
                ++new_deque_it;
            }
            --new_deque_it;
        }
        catch(...){
            --new_deque_it;
            end_pos = --iterator(new_buckets_ptr, new_buckets_capacity, new_buckets_ptr, *new_buckets_ptr);
            while (new_deque_it != end_pos){
                std::allocator_traits<Allocator>::destroy(m_alloc, new_deque_it.m_ptr);
                --new_deque_it;
            }
            for(decltype(success_allocated_count) i = 0; i < success_allocated_count; ++i){
                std::allocator_traits<Allocator>::deallocate(m_alloc, new_buckets_ptr[i], BucketSize);
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, new_buckets_ptr, new_buckets_capacity);
            throw;
        }
        size_t temp_size = m_size;
        clear();
        m_size = temp_size;
        
        T** end_bucket_pos = m_last_allocated_bucket_ptr + 1;
        while(m_first_allocated_bucket_ptr != end_bucket_pos){
            std::allocator_traits<Allocator>::deallocate(m_alloc, *m_first_allocated_bucket_ptr, BucketSize);
            ++m_first_allocated_bucket_ptr;
        }
        std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, m_buckets_capacity);
        
        m_buckets_ptr = new_buckets_ptr;
        m_buckets_capacity = new_buckets_capacity;

        m_first_allocated_bucket_ptr = new_buckets_ptr;
        m_last_allocated_bucket_ptr  = new_buckets_ptr + new_buckets_capacity - 1;

        m_first.m_buckets_ptr = new_buckets_ptr;
        m_first.m_buckets_capacity = new_buckets_capacity;
        m_first.m_bucket_ptr = m_first_allocated_bucket_ptr;
        m_first.m_ptr = *m_first_allocated_bucket_ptr;

        m_last = new_deque_it;
    }



    void clear() {
        erase(cbegin(), cend());
    }
    

    // iterator insert(const_iterator pos, const T& value){}
    
    // iterator insert(const_iterator pos, T&& value){}
    

    iterator insert(const_iterator pos, size_type count, const T& value){
        if (count < 1) {return {pos.m_buckets_ptr, pos.m_buckets_capacity, pos.m_bucket_ptr, const_cast<T*>(pos.m_ptr)};}

        else if (m_buckets_ptr == nullptr){
            size_t count_of_needed_buckets = (count % BucketSize == 0) ? count / BucketSize : (count / BucketSize) + 1;

            m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, count_of_needed_buckets);
            size_t i = 0;
            try{ // strong exception guarantee
                for (;i < count_of_needed_buckets; ++i)
                    m_buckets_ptr[i] = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
            }
            catch(...){
                while (i > 0){
                    --i;
                    std::allocator_traits<Allocator>::deallocate(m_alloc, m_buckets_ptr[i], BucketSize);
                }
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, 1);
                m_buckets_ptr = nullptr;
                throw; 
            }

            i = 0;
            size_t j = 0;
            size_t successful_constructed_count = 0;
            try{ // strong exception guarantee
                for(;i < count_of_needed_buckets; ++i){
                    for(j = 0; (j < BucketSize) && (successful_constructed_count < count); ++j, ++successful_constructed_count){
                        std::allocator_traits<Allocator>::construct(m_alloc, m_buckets_ptr[i] + j, value);
                    }
                }
            }
            catch(...){
                i = 0;
                for(;i < count_of_needed_buckets; ++i){
                    for(j = 0; (j < BucketSize) && (successful_constructed_count > 0); ++j, --successful_constructed_count){
                        std::allocator_traits<Allocator>::destroy(m_alloc, m_buckets_ptr[i] + j);
                    }
                    std::allocator_traits<Allocator>::deallocate(m_alloc, m_buckets_ptr[i], BucketSize);
                }
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, 1);
                m_buckets_ptr = nullptr;
                throw; 
            }
        
            m_buckets_capacity = count_of_needed_buckets;
            
            m_first_allocated_bucket_ptr = m_buckets_ptr;
            m_last_allocated_bucket_ptr = m_buckets_ptr + count_of_needed_buckets - 1;

            m_first.m_buckets_ptr = m_buckets_ptr;
            m_first.m_buckets_capacity = m_buckets_capacity;
            m_first.m_bucket_ptr = m_first_allocated_bucket_ptr;
            m_first.m_ptr = *m_first.m_bucket_ptr;
            
            m_last.m_buckets_ptr = m_buckets_ptr;
            m_last.m_buckets_capacity = m_buckets_capacity;
            m_last.m_bucket_ptr = m_last_allocated_bucket_ptr;
            m_last.m_ptr = *m_last_allocated_bucket_ptr + j - 1; // after cycle of constructing elements, j is incremented by 1 more, than necessary (to stop cycle)

            m_size += count;

            return {m_first};
        }
        else if (m_size == 0){ // m_first & m_last are centered in deque and pointing to the same position (call center_the_iterators_m_first_and_m_last_())
            size_t count_of_allocated_cells = (m_last_allocated_bucket_ptr - m_first_allocated_bucket_ptr + 1) * BucketSize;
            if (count_of_allocated_cells >= count){
                size_t new_pos_for_m_first = (count_of_allocated_cells - count) / 2;
                // set m_first in the beginning
                m_first.m_bucket_ptr = m_first_allocated_bucket_ptr;
                m_first.m_ptr = *m_first.m_bucket_ptr;
                
                m_first += new_pos_for_m_first;
                m_last = m_first;
                size_t successful_constructed_count = 0;
                try{ //strong exception safety
                    while(successful_constructed_count < count){
                        std::allocator_traits<Allocator>::construct(m_alloc, m_last.m_ptr, value);
                        ++m_last;
                        ++successful_constructed_count;
                    }
                    //now m_last is equal to end(), that's why:
                    --m_last;
                }
                catch(...){
                    --m_last;
                    while(successful_constructed_count > 0){
                        std::allocator_traits<Allocator>::destroy(m_alloc, m_last.m_ptr);
                        --m_last;
                        --successful_constructed_count;
                    }
                    center_the_iterators_m_first_and_m_last_();
                    throw;
                }
                m_size += count;
                return m_first;
            }
            else { // we need to allocate additional buckets
                size_t possible_count_of_cells = m_buckets_capacity * BucketSize;
                if (possible_count_of_cells >= count){
                    size_t total_count_of_needed_buckets = ((count % BucketSize == 0) ? (count / BucketSize) : ((count / BucketSize) + 1));
                    size_t start_index = (m_buckets_capacity - total_count_of_needed_buckets) / 2;
                /*  
                    start_index --> the position in the outer array from which bucket allocation should begin, so that after all the 
                    required buckets are allocated from both edges of the deck, there remains (approximately) equal unallocated space
                */
                    size_t count_of_lack_buckets = total_count_of_needed_buckets - ((m_last_allocated_bucket_ptr - m_first_allocated_bucket_ptr) + 1); 
                    size_t count_of_successful_allocated_buckets = 0;
                    
                    T** begin_bound_ptr = m_buckets_ptr + start_index;
                    T** current_new_bucket_ptr = m_first_allocated_bucket_ptr - 1;
                    // cuurent_new_backet_ptr will bw stepping from (m_first_allocated_bucets_ptr - 1) to begin_bound_ptr

                    try{ //strong exception guarantee
                        while (current_new_bucket_ptr >= begin_bound_ptr && count_of_successful_allocated_buckets < count_of_lack_buckets){
                            *current_new_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                            --current_new_bucket_ptr;
                            ++count_of_successful_allocated_buckets;
                        }
                        begin_bound_ptr = current_new_bucket_ptr + 1;
                    /*
                        why we need to update bwgin_bound?
                        
                        in some cases, distance between (m_first_allocated_bucket_ptr) and (begin_bound) can be more then (count_of_lack_buckets),
                        and then, (current_new_bucket_ptr) won`t reach begin_bount -> begin_bount may will be unallocated;
                        Further, begin_bound is use instead of (m_first_allocated_bucket_ptr) as the first allocated bucket. 
                    */
                        
                        if (count_of_successful_allocated_buckets < count_of_lack_buckets){
                            current_new_bucket_ptr = m_last_allocated_bucket_ptr + 1;
                        }
                        else{
                            current_new_bucket_ptr = begin_bound_ptr;
                        }
                        while(count_of_successful_allocated_buckets < count_of_lack_buckets){
                        // in the th cycle, (count_of_successful_allocated_buckets) will never go outside the outer array 
                            *current_new_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                            ++current_new_bucket_ptr;
                            ++count_of_successful_allocated_buckets;
                        }
                    } 
                    catch(...){
                        if (current_new_bucket_ptr > m_last_allocated_bucket_ptr){
                            --current_new_bucket_ptr;
                            while(current_new_bucket_ptr != m_last_allocated_bucket_ptr){ 
                            // in the th cycle, (count_of_successful_allocated_buckets) will never reach 0
                                std::allocator_traits<Allocator>::deallocate(m_alloc, *current_new_bucket_ptr, BucketSize);
                                --current_new_bucket_ptr;
                                --count_of_successful_allocated_buckets;
                            }
                        }
                        m_last_allocated_bucket_ptr = m_first_allocated_bucket_ptr - 1;
                        while(count_of_successful_allocated_buckets != 0){                  
                            std::allocator_traits<Allocator>::deallocate(m_alloc, *current_new_bucket_ptr, BucketSize);
                            --current_new_bucket_ptr;
                            --count_of_successful_allocated_buckets;
                        }
                        throw;
                    }
                   
                    // constructing element
                    iterator current_it{m_buckets_ptr, m_buckets_capacity, begin_bound_ptr, *begin_bound_ptr};
                    size_t successful_constructed_count = 0;
                    try{ //strong exception safety
                        while(successful_constructed_count < count){
                            std::allocator_traits<Allocator>::construct(m_alloc, current_it.m_ptr, value);
                            ++current_it;
                            ++successful_constructed_count;
                        }
                        //now m_last is equal to end(), that's why:
                        --current_it;
                    }
                    catch(...){
                        --current_it;
                        while(successful_constructed_count > 0){
                            std::allocator_traits<Allocator>::destroy(m_alloc, current_it.m_ptr);
                            --current_it;
                            --successful_constructed_count;
                        }
                        
                        // free new allocated needed buckets;
                        if (current_new_bucket_ptr > m_last_allocated_bucket_ptr){
                            --current_new_bucket_ptr;
                            while(current_new_bucket_ptr != m_last_allocated_bucket_ptr){ 
                            // in the th cycle, (count_of_successful_allocated_buckets) will never reach 0
                                std::allocator_traits<Allocator>::deallocate(m_alloc, *current_new_bucket_ptr, BucketSize);
                                --current_new_bucket_ptr;
                                --count_of_successful_allocated_buckets;
                            }
                        }
                        current_new_bucket_ptr = m_first_allocated_bucket_ptr - 1;
                        while(count_of_successful_allocated_buckets > 0){ 
                            std::allocator_traits<Allocator>::deallocate(m_alloc, *current_new_bucket_ptr, BucketSize);
                            --current_new_bucket_ptr;
                            --count_of_successful_allocated_buckets;
                        } 
                        throw;
                    }
                    
                    m_first.m_bucket_ptr = begin_bound_ptr;
                    m_first.m_ptr = *m_first.m_bucket_ptr;
                    
                    m_last = current_it;

                    m_first_allocated_bucket_ptr = m_buckets_ptr + start_index;
                    m_last_allocated_bucket_ptr = current_new_bucket_ptr - 1;
                    
                    m_size += count;
                    
                    return m_first; 
                }
                else{

                    // realloc outer array
                    // allocating additional buckets
                    // constructing elements
                }
            }
        }

        else{ // m_size != 0
            if (m_last - pos <= pos - m_first){ 
            // shift elements to the end side

                size_t count_of_allocated_cells_from_m_last = 
                    ((m_last_allocated_bucket_ptr - m_last.m_bucket_ptr) * BucketSize)
                    +
                    ((*m_last.m_bucket_ptr + BucketSize - 1) - m_last.m_ptr);

                if (count_of_allocated_cells_from_m_last >= count){
                    // shift old elements
                    // constructing new elements
                }
                else { // lack of allocated cells in the end side. Let's try moving empty allocated backets from the beginning to the end.
                    size_t count_of_lacking_cells = count - count_of_allocated_cells_from_m_last; 
                    size_t count_of_free_allocated_buckets_in_the_beginning = m_first.m_bucket_ptr - m_first_allocated_bucket_ptr;
                    
                    if ((count_of_free_allocated_buckets_in_the_beginning * BucketSize) >= count_of_lacking_cells){
                        // try to swap free alllocated buckets from the beginning to the end
                        // shift old elements
                        // constructing new elements
                    }
                    else{ // lack of allocated buckets -> we need to allocate new buckets
                        size_t count_of_non_allocated_buckets_in_the_end = (m_buckets_ptr + m_buckets_capacity - 1) - m_last_allocated_bucket_ptr;
                        if ((count_of_non_allocated_buckets_in_the_end * BucketSize) >= count_of_lacking_cells){
                            // allocating lacking buckets 
                            // shift old elements
                            // constructing new elements
                        }
                        else{
                            // reallocate outer array 
                            // allocate lacking buckets 
                            // shift old elements
                            // constructing new elements
                        }
                    }
                } // end else {...} -> lack of allocated cells in the end side.
            } 
            else { 
            // shift elements to the beginning side
                size_t count_of_allocated_cells_from_m_first = 
                    ((m_first.m_bucket_ptr - m_first_allocated_bucket_ptr) * BucketSize)
                    +
                    (m_first.m_ptr - *m_first.m_bucket_ptr);
                
                if (count_of_allocated_cells_from_m_first >= count){
                    // shift old elements
                    // constructing new elements
                }
                else { // lack of allocated cells in the beginning side. Let's try moving empty allocated backets from the end to the beginning.
                    size_t count_of_lacking_cells = count - count_of_allocated_cells_from_m_first; 
                    size_t count_of_free_allocated_buckets_in_the_end = m_last_allocated_bucket_ptr - m_last.m_bucket_ptr;

                    if ((count_of_free_allocated_buckets_in_the_end * BucketSize) >= count_of_lacking_cells){
                        // try to swap free alllocated buckets from the beginning to the end
                        // shift old elements
                        // constructing new elements
                    }
                    else{ // lack of allocated buckets -> we need to allocate new buckets
                        size_t count_of_non_allocated_buckets_in_the_beginning = m_first_allocated_bucket_ptr - m_buckets_ptr;
                        if ((count_of_non_allocated_buckets_in_the_beginning * BucketSize) >= count_of_lacking_cells){
                            // allocating lacking buckets 
                            // shift old elements
                            // constructing new elements
                        }
                        else{
                            // reallocate outer array 
                            // allocate lacking buckets 
                            // shift old elements
                            // constructing new elements
                        }
                    }
                }
            } // end else {...} -> shift to the beginning
        } // end of else (m_size != 0) {...}
        return {pos.m_buckets_ptr, pos.m_buckets_capacity, pos.m_bucket_ptr, const_cast<T*>(pos.m_ptr)};
    }

    
    /*
    template <typename InputIt> 
    iterator insert(const_iterator pos, InputIt first, InputIt last){}
    */
    
    //iterator insert(std::initializer_list<T> init_list){}
    
    /*
    template< class... Args >
    iterator emplace( const_iterator pos, Args&&... args );
    */


    //iterator erase( const_iterator pos );



    iterator erase(const_iterator first, const_iterator last){
        if (m_size == 0) {return end();}

        if (first == last) {    
            return {last.m_buckets_ptr, last.m_buckets_capacity, last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
        } 
        /*
            like in gcc & clang here is no checking (first > last);
            it means that if (first > last) -> UB
        */
        if (first == m_first){
            if(last == cend()){ 
                while(m_first != last){
                    std::allocator_traits<Allocator>::destroy(m_alloc, m_first.m_ptr);
                    ++m_first;
                }

                // for that future inserts are inside the middle of the deck:
                m_first.m_bucket_ptr = m_last.m_bucket_ptr = m_buckets_ptr + (m_buckets_capacity / 2);
                m_first.m_ptr = m_last.m_ptr = *m_last.m_bucket_ptr;
                m_size = 0;
                center_the_iterators_m_first_and_m_last_();
            }
            else{
                while(m_first != last){
                    std::allocator_traits<Allocator>::destroy(m_alloc, m_first.m_ptr);
                    ++m_first;
                    --m_size;
                }
            } 
            return {last.m_buckets_ptr, last.m_buckets_capacity, last.m_bucket_ptr, const_cast<T*>(last.m_ptr)}; 
        }
        else if (last == cend()){
            const_iterator end_pos = first - 1;
            while(m_last != end_pos){
                std::allocator_traits<Allocator>::destroy(m_alloc, m_last.m_ptr);
                --m_last;
                --m_size;
            }
            return {last.m_buckets_ptr, last.m_buckets_capacity, last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
        }

        else if (first.m_ptr == *first.m_bucket_ptr && last.m_ptr == *last.m_bucket_ptr){

            //we move the delete bucket to the deque boundary to avoid unnecessary element movements
            //we check in which of halves is delete bucket to move him to the nearest border to reduce count of swaps
            difference_type count_delete_buckets = last.m_bucket_ptr - first.m_bucket_ptr;
            if ((m_last.m_bucket_ptr - last.m_bucket_ptr  + 1) <= (first.m_bucket_ptr - m_first.m_bucket_ptr)){
                //move to end

                difference_type old_distance_from_last_to_end = m_last.m_bucket_ptr - last.m_bucket_ptr;
                /*
                difference_type old_distance_from_last_to_end:

                The point is that after the delete_buckets move is complete, the last.m_bucket_ptr pointer, which has type (T**),
                continues to point to its bucket_array position, but after moving the pointers on buckets, the last.m_bucket_ptr 
                pointer becomes invalid;
                So to keep the iterator to the real first non-deletable element, we keep the distance from its bucket to the 
                last bucket.
                IMPORTANT: the last.m_ptr pointer remains valid after all bucket moves, since we don't move the real buckets 
                themselves, but only the pointers to them.            
            */
                // As long as the loop condition is true, we can completely move the block of pointers_to_delete_buckets to the size of this block.
                while (m_last.m_bucket_ptr - (first.m_bucket_ptr + count_delete_buckets - 1) >= count_delete_buckets){
                    for (difference_type i = count_delete_buckets - 1; i >= 0; --i){
                        std::swap(first.m_bucket_ptr[i], first.m_bucket_ptr[i+count_delete_buckets]);
                    }
                    first.m_bucket_ptr +=count_delete_buckets; 
                }
                // now the distance between the last_bucket of the deck and the bucket block of pointers_to_delete_buckets is less than the size of the bucket block
                difference_type remainder_size = m_last.m_bucket_ptr - first.m_bucket_ptr + count_delete_buckets - 1;

                for (difference_type i = 0; i < remainder_size; ++i){
                    std::swap(first.m_bucket_ptr[i], first.m_bucket_ptr[i+count_delete_buckets]);
                }  
                first.m_bucket_ptr += remainder_size; // first.m_bucket_ptr is pointing on first trash_bucket
                first.m_ptr = *first.m_bucket_ptr; // updating m_ptr
                m_last.m_bucket_ptr = first.m_bucket_ptr - 1;

                T** end_pos = first.m_bucket_ptr + count_delete_buckets;
                while(first.m_bucket_ptr != end_pos){
                    std::allocator_traits<Allocator>::destroy(m_alloc, const_cast<T*>(first.m_ptr));
                    ++first; 
                }       

                return {m_buckets_ptr, m_buckets_capacity, m_last.m_bucket_ptr - old_distance_from_last_to_end, const_cast<T*>(last.m_ptr)};
            }

            // move to begin
            while (first.m_bucket_ptr - m_first.m_bucket_ptr >= count_delete_buckets){
                for (difference_type i = count_delete_buckets - 1; i >= 0; --i){
                    std::swap(first.m_bucket_ptr[i], first.m_bucket_ptr[i - count_delete_buckets]);
                }
                first.m_bucket_ptr -= count_delete_buckets; 
            }
            difference_type reminder_size = first.m_bucket_ptr - m_first.m_bucket_ptr;

            for (difference_type i = 0; i < reminder_size; ++i){
                std::swap(first.m_bucket_ptr[-i + count_delete_buckets - 1], first.m_bucket_ptr[-i-1]);
            } 

            first.m_bucket_ptr -=  reminder_size; // first.m_bucket_ptr is pointing on first trach_bucket
            m_first.m_bucket_ptr = first.m_bucket_ptr + count_delete_buckets; 

            while(first.m_bucket_ptr != m_first.m_bucket_ptr){
                std::allocator_traits<Allocator>::destroy(m_alloc, const_cast<T*>(first.m_ptr));
                ++first; 
            }        
            return {last.m_buckets_ptr, last.m_buckets_capacity, last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
        }

        //the worst case

        //Because, const_iterator::operator* returns const T& than we need to get a non const iterator to first (to avoid copy instead move)
        iterator first_it(first.m_buckets_ptr, first.m_buckets_capacity, first.m_bucket_ptr, const_cast<T*>(first.m_ptr));
        iterator second_it(last.m_buckets_ptr, last.m_buckets_capacity, last.m_bucket_ptr, const_cast<T*>(last.m_ptr));

        if (m_last - last < first - m_first){
        // move delet-elements to end side
            
            iterator return_pos = first_it;
            iterator end_pos = end();
            while(second_it != end_pos){
                std::swap(*first_it, *second_it);
                ++first_it;
                ++second_it;
            } 
            // there, first_it points on the first trash-element;
            --first_it; // there, first_it points on the new last_;
            while(m_last != first_it){
                std::allocator_traits<Allocator>::destroy(m_alloc, m_last.m_ptr);
                --m_last;
                --m_size;
            }
            return return_pos;
        }

        // move to the begin side 
        --first_it;
        --second_it;
        iterator end_pos = begin() - 1;
        while(first_it != end_pos){
            std::swap(*first_it, *second_it);
            --first_it;
            --second_it;
        }
        // second_it points on the last trash-elemnt;
        ++second_it; // second_it points on the new first_;
        while(m_first != second_it){
            std::allocator_traits<Allocator>::destroy(m_alloc, m_first.m_ptr);
            ++m_first;
            --m_size;
        }

        return {last.m_buckets_ptr, last.m_buckets_capacity, last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
    }


    void push_back( const T& value ){ 
        if (!m_buckets_ptr){
            auto result_of_realloc = realloc_with_add_allocated_buckets_to_end(1); 
             
            try{ //strong exception safety
                std::allocator_traits<Allocator>::construct(m_alloc, result_of_realloc.new_m_last.m_ptr, value);
            }
            catch(...){
                std::allocator_traits<Allocator>::deallocate(m_alloc, *result_of_realloc.new_m_last.m_bucket_ptr, BucketSize);
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, result_of_realloc.new_m_buckets_ptr, result_of_realloc.new_m_buckets_capacity);
                throw;
            }
            m_buckets_ptr = result_of_realloc.new_m_buckets_ptr;
            m_buckets_capacity = result_of_realloc.new_m_buckets_capacity;

            m_first_allocated_bucket_ptr = result_of_realloc.new_m_first_allocated_bucket_ptr;
            m_last_allocated_bucket_ptr  = result_of_realloc.new_m_last_allocated_bucket_ptr;


            m_first.m_buckets_ptr = m_buckets_ptr;
            m_first.m_buckets_capacity = m_buckets_capacity;
            m_first.m_bucket_ptr = m_first_allocated_bucket_ptr;
            m_first.m_ptr = *m_first_allocated_bucket_ptr; 

            m_last = m_first;
            
            ++m_size;
            return;
        }
        else if (m_size == 0){
            std::allocator_traits<Allocator>::construct(m_alloc, m_last.m_ptr, value);
            ++m_size;
            return;
        }
        else if ((m_last.m_ptr - *m_last.m_bucket_ptr) < static_cast<long int>(BucketSize - 1)){
            std::allocator_traits<Allocator>::construct(m_alloc, m_last.m_ptr + 1, value);
            ++m_last.m_ptr;
        /*
            it is not appropriate to increment the entire iterator (++m_last) here, since the condition 
            satisfied guarantees that (++m_last) will not require a transition to the next bucket
        */
            ++m_size;
            return;
        }
        else {
            if ((m_last.m_bucket_ptr - m_buckets_ptr) < static_cast<long int>(m_buckets_capacity - 1)){
                if(m_last.m_bucket_ptr != m_last_allocated_bucket_ptr){
                    std::allocator_traits<Allocator>::construct(m_alloc, *(m_last.m_bucket_ptr + 1), value);
                }
                else{
                    *(m_last_allocated_bucket_ptr + 1) = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                    try{ //strong exception safety
                        std::allocator_traits<Allocator>::construct(m_alloc, *(m_last_allocated_bucket_ptr + 1), value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(m_alloc, *(m_last_allocated_bucket_ptr + 1), BucketSize);
                        throw;
                    }
                    ++m_last_allocated_bucket_ptr;
                }
                ++m_last;
                ++m_size;
                return;
            }
            else{ // the worst case --> need reallocation
                auto result_of_realloc = realloc_with_add_allocated_buckets_to_end(1); 
                ++result_of_realloc.new_m_last;
                try{ //strong exception safety
                    std::allocator_traits<Allocator>::construct(m_alloc, result_of_realloc.new_m_last.m_ptr, value);
                }
                catch(...){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *result_of_realloc.new_m_last.m_bucket_ptr, BucketSize);
                    std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, result_of_realloc.new_m_buckets_ptr, result_of_realloc.new_m_buckets_capacity);
                    throw;
                }
            
                m_first = result_of_realloc.new_m_first;
                m_last  = result_of_realloc.new_m_last;
                
                m_first_allocated_bucket_ptr = result_of_realloc.new_m_first_allocated_bucket_ptr;
                m_last_allocated_bucket_ptr = result_of_realloc.new_m_last_allocated_bucket_ptr;
                
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, m_buckets_capacity);
                m_buckets_ptr = result_of_realloc.new_m_buckets_ptr;
                m_buckets_capacity = result_of_realloc.new_m_buckets_capacity;
                ++m_size;
                return;
            }    
        }
    }


    
    //void push_back( T&& value ){}



    /*
    template< class... Args >
    reference emplace_back( Args&&... args );
    */


    void pop_back(){
        if (m_size == 0){return;}
        std::allocator_traits<Allocator>::destroy(m_alloc, m_last.m_ptr);
        --m_last;
        --m_size;
        if (m_size == 0){
            center_the_iterators_m_first_and_m_last_();
        }
    }


    //void push_front( const T& value );

    //void push_front( T&& value );


    /*
    template< class... Args >
    reference emplace_front( Args&&... args );
    */


    void pop_front(){
        if (m_size == 0){return;}
        std::allocator_traits<Allocator>::destroy(m_alloc, m_first.m_ptr);
        ++m_first;
        --m_size;
        if (m_size == 0){
            center_the_iterators_m_first_and_m_last_();
        }
    }
    

    //void resize( size_type count );

    //void resize( size_type count, const value_type& value );

    //void swap( deque& other ) noexcept(noexcept(std::allocator_traits<Allocator>::is_always_equal::value));

};


template<typename T, size_t BucketSize, typename Allocator = std::allocator<T>>
using deque_dimensional = deque<T, Allocator, BucketSize>;

} // end namespace Farebl
#endif // FAREBL_DEQUE_H
