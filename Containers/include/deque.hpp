
#ifndef FAREBL_DEQUE_H
#define FAREBL_DEQUE_H

#include <memory>
#include <limits>
#include <cmath>

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
        pointer m_ptr;
        
        difference_type m_bucket_index; 
        size_t m_cell_index; 
    /*      
        invalid iterator --> out-of-range iterator;

        m_pseudo_bucket_index:
        it is neede ONLY for invalid iterator

        m_cell_index: 
        if (m_cell_index == -1) --> it means that it is valid iterator
        else (invalid iterator) --> it means index of (pseudo-cell), in pseudo bucket
    */


        base_iterator():
            m_buckets_ptr(nullptr), 
            m_buckets_capacity(0),
            m_ptr(nullptr),
            m_bucket_index(0),
            m_cell_index(0){}

        base_iterator(T** buckets_ptr, size_t buckets_capacity, pointer ptr, difference_type bucket_index, size_t cell_index):  
            m_buckets_ptr(buckets_ptr), 
            m_buckets_capacity(buckets_capacity),
            m_ptr(ptr),
            m_bucket_index(bucket_index),
            m_cell_index(cell_index){}

    public:

        reference operator*() const {return *m_ptr; }

	    pointer operator->() const {return m_ptr;}

        base_iterator& operator++(){
            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    if((m_cell_index + 1) == static_cast<difference_type>(BucketSize)){
                        ++m_bucket_index;
                        if (
                            (m_bucket_index >= 0)
                                &&
                            (m_bucket_index < m_buckets_capacity)
                        ){
                            m_ptr = m_buckets_ptr[m_bucket_index];
                        }
                        else{
                            m_ptr = nullptr; 
                        }  
                        m_cell_index = 0; // iterator points of 1-st pseudo cell of pseudo bucket
                    }
                    else{
                        ++m_ptr; 
                        ++m_cell_index; // iterator points of 1-st pseudo cell of pseudo bucket
                    }
                }
                else {
                    if((m_cell_index + 1) == static_cast<difference_type>(BucketSize)){
                        ++m_bucket_index;
                        if (
                            (m_bucket_index >= 0)
                                &&
                            (m_bucket_index < m_buckets_capacity)
                        ){
                            m_ptr = m_buckets_ptr[m_bucket_index];
                        }
                        m_cell_index = 0;
                    }
                    else{
                        ++m_cell_index;
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
                    if (m_cell_index == 0){
                        --m_bucket_index;
                        m_cell_index = BucketSize - 1;
                        if (
                            (m_bucket_index >= 0)
                                &&
                            (m_bucket_index < m_buckets_capacity)
                        ){
                            m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                        }
                        else{
                            m_ptr = nullptr;
                        }
                    }   
                    else{
                        --m_ptr;
                        --m_cell_index;
                    }
                }
                else{
                    if(m_cell_index == 0){
                        --m_bucket_index;
                        m_cell_index = BucketSize - 1;
                        if (
                            (m_bucket_index >= 0)
                                &&
                            (m_bucket_index < m_buckets_capacity)
                        ){
                            m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                        }
                    }
                    else{
                        --m_cell_index;
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
            if (value < 0) {return *this -= std::abs(value);}

            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    difference_type result_index = m_cell_index + (value % BucketSize);
                    
                    m_bucket_index += value / BucketSize;
                    if (
                        (m_bucket_index >= 0)
                            &&
                        (m_bucket_index < m_buckets_capacity)
                    ){ 
                        if (result_index < static_cast<difference_type>(BucketSize)){
                            m_cell_index = result_index;
                            m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                        }
                        else{
                            ++m_bucket_index;
                            m_cell_index = result_index - static_cast<difference_type>(BucketSize); 
                            if (
                                (m_bucket_index >= 0)
                                    &&
                                (m_bucket_index < m_buckets_capacity)
                            ){ 
                                m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                            }
                            else{
                                m_ptr = nullptr;
                            }
                        }
                    }
                    else{
                        m_ptr = nullptr;
                        if (result_index < static_cast<difference_type>(BucketSize)){
                            m_cell_index = result_index;
                        }
                        else{
                            ++m_bucket_index;
                            // there isn`t necessary to check boundaries: we have already gone beyond them
                            m_cell_index = result_index - static_cast<difference_type>(BucketSize);
                        }
                    }
                }
                else{
                    difference_type result_pseudo_index = m_cell_index + (value % BucketSize);
                    if (value >= static_cast<difference_type>(BucketSize)){
                        m_bucket_index += value / BucketSize;
                    }
                    if (
                        (m_bucket_index >= 0)
                            &&
                        (m_bucket_index < m_buckets_capacity)
                    ){
                        if (result_pseudo_index < static_cast<difference_type>(BucketSize)){
                            difference_type result_index = m_cell_index + (value % BucketSize);
                            m_cell_index = result_index; 
                            m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                        }
                        else{
                            ++m_bucket_index;
                            m_cell_index = result_pseudo_index - static_cast<difference_type>(BucketSize);
                            if (
                                (m_bucket_index >= 0)
                                    &&
                                (m_bucket_index < m_buckets_capacity)
                            ){ 
                                m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                            }
                        }
                    }
                    else{
                        if (result_pseudo_index < static_cast<difference_type>(BucketSize)){
                            m_cell_index = result_pseudo_index;
                        }
                        else{
                            ++m_bucket_index;
                            m_cell_index = result_pseudo_index - static_cast<difference_type>(BucketSize);
                            if (
                                (m_bucket_index >= 0)
                                    &&
                                (m_bucket_index < m_buckets_capacity)
                            ){
                                m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                            }
                        }
                    }
                }
            }
            return *this;
        }

        base_iterator& operator-=(difference_type value) & {
            if (value < 0) {return *this += std::abs(value);}

            if (m_buckets_ptr != nullptr){
                if (m_ptr != nullptr){
                    difference_type result_index_in_bucket = m_cell_index - (value % BucketSize);
                    
                    m_bucket_index -= value / BucketSize;
                    if (
                        (m_bucket_index >= 0)
                            &&
                        (m_bucket_index < m_buckets_capacity)
                    ){ 
                        if (result_index_in_bucket > -1){
                            m_cell_index = result_index_in_bucket;
                            m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                        } 
                        else{
                            --m_bucket_index;
                            m_cell_index = BucketSize + result_index_in_bucket;
                            if (
                                (m_bucket_index >= 0)
                                    &&
                                (m_bucket_index < m_buckets_capacity)
                            ){
                                m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                            }
                            else{
                                m_ptr = nullptr;
                            }
                        }
                    }
                    else{
                        m_ptr = nullptr;
                        if (result_index_in_bucket > -1){
                            m_cell_index = result_index_in_bucket;
                        } 
                        else{
                            --m_bucket_index;
                            m_cell_index = BucketSize + result_index_in_bucket;
                        }
                    }
                }
                else {
                    difference_type result_pseudo_index_in_bucket = m_cell_index - (value % BucketSize);
                    m_bucket_index -= value / BucketSize;
                    if (
                        (m_bucket_index >= 0)
                            &&
                        (m_bucket_index < m_buckets_capacity)
                    ){
                        if (result_pseudo_index_in_bucket > -1){
                            m_cell_index = result_pseudo_index_in_bucket;
                            m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                        } 
                        else{
                            --m_bucket_index;
                            m_cell_index = (BucketSize + result_pseudo_index_in_bucket);
                            if (
                                (m_bucket_index >= 0)
                                    &&
                                (m_bucket_index < m_buckets_capacity)
                            ){
                                m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                            }
                        }
                    }
                    else{
                        if (result_pseudo_index_in_bucket > -1){
                             m_cell_index = result_pseudo_index_in_bucket;
                        } 
                        else{
                            --m_bucket_index;
                            m_cell_index = (BucketSize + result_pseudo_index_in_bucket);
                            if (
                                (m_bucket_index >= 0)
                                    &&
                                (m_bucket_index < m_buckets_capacity)
                            ){
                                m_ptr = m_buckets_ptr[m_bucket_index] + m_cell_index;
                            }
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
            difference_type distance = 0;
            if (m_bucket_index == other.m_bucket_index){
                distance += m_cell_index - other.m_cell_index;
            }
            else{
                if ((*this) > other){
                    distance += ((m_bucket_index - other.m_bucket_index) - 1) * BucketSize;
                    distance += m_cell_index + (BucketSize - other.m_cell_index);
                }
                else{
                    distance -= ((other.m_bucket_index - m_bucket_index) - 1) * BucketSize;
                    distance -= other.m_cell_index + (BucketSize - m_cell_index);
                }
            }return distance;
        }


        base_iterator& operator[](size_t index){return *(*this+index);}


        template<bool OtherIsConst>
        bool operator==(const base_iterator<OtherIsConst>& other) const {
            return (
                (m_buckets_ptr == other.m_buckets_ptr)
                    &&
                (m_buckets_capacity == other.m_buckets_capacity)
                    &&
                (m_bucket_index == other.m_bucket_index)
                    && 
                (m_ptr == other.m_ptr)
            ); 
        }

        template<bool OtherIsConst>
        bool operator!=(const base_iterator<OtherIsConst>& other) const {
            return !(*this == other);
        }

        template<bool OtherIsConst>
        bool operator<(const base_iterator<OtherIsConst>& other) const { 
            return (m_bucket_index < other.m_bucket_index) ? true : (m_bucket_index == other.m_bucket_index && m_cell_index < other.m_cell_index) ? true : false; 
        }
        
        template<bool OtherIsConst>
        bool operator>(const base_iterator<OtherIsConst>& other) const {return other < *this;}

        template<bool OtherIsConst>
        bool operator>=(const base_iterator<OtherIsConst>& other) const {return !(*this < other); }

        template<bool OtherIsConst>
        bool operator<=(const base_iterator<OtherIsConst>& other) const {return !(*this > other); }

        operator base_iterator<true>(){return {m_buckets_ptr, m_buckets_capacity, const_cast<const T*>(m_ptr), m_bucket_index, m_cell_index};}
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
    size_t m_first_allocated_bucket_index;
    size_t m_last_allocated_bucket_index;
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
        size_t index_of_middle_allocated_bucket = m_first_allocated_bucket_index + ((m_last_allocated_bucket_index - m_first_allocated_bucket_index) / 2);

        m_last.m_bucket_index = index_of_middle_allocated_bucket;
        m_last.m_cell_index = 0;
        m_last.m_ptr = m_buckets_ptr[m_last.m_bucket_index];
        m_first = m_last; 
    }

    struct NewPtrsAndCapAfterRealloc{
        T** new_m_buckets_ptr; 
        difference_type new_m_first_allocated_bucket_index;
        difference_type new_m_last_allocated_bucket_index;
        iterator new_m_first;
        iterator new_m_last;
        size_t new_m_buckets_capacity;
    };
    

    NewPtrsAndCapAfterRealloc realloc_with_add_allocated_buckets_to_the_beginning(size_t count_of_buckets, bool to_add_a_reserve){
        NewPtrsAndCapAfterRealloc result;
        if(!m_buckets_ptr){
            result.new_m_buckets_capacity = count_of_buckets;
            result.new_m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, result.new_m_buckets_capacity);
            
            T** new_first_allocated_bucket_ptr = result.new_m_buckets_ptr;
            T** new_last_allocated_bucket_ptr = result.new_first_allocated_bucket_ptr;
            
            try{
                for (size_t successful_allocated_buckets = 0; successful_allocated_buckets < count_of_buckets; ++successful_allocated_buckets, ++new_last_allocated_bucket_ptr){
                    *new_last_allocated_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                }
                --new_last_allocated_bucket_ptr;
            }
            catch(...){
                new_last_allocated_bucket_ptr;
                T** end_pos = result.new_m_buckets_ptr - 1;
                while(new_last_allocated_bucket_ptr != end_pos){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *new_last_allocated_bucket_ptr, BucketSize);
                    --new_last_allocated_bucket_ptr;
                }
                throw;
            }
            result.new_m_first_allocated_bucket_index = new_first_allocated_bucket_ptr - result.new_m_buckets_ptr;
            result.new_m_last_allocated_bucket_index = new_last_allocated_bucket_ptr - result.new_m_buckets_ptr;

            result.new_m_first.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_first.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_first.m_bucket_index = result.new_m_first_allocated_bucket_index;
            result.new_m_first.m_cell_index = BucketSize - 1;
            result.new_m_first.m_ptr = result.new_m_buckets_ptr[result.new_m_first.m_bucket_index] + result.new_m_first.m_cell_index;

            result.new_m_last = result.new_m_first;
        }
        else{
            size_t old_count_of_allocated_buckets = m_last_allocated_bucket_index - m_first_allocated_bucket_index + 1;
            result.new_m_buckets_capacity = old_count_of_allocated_buckets + count_of_buckets;

            size_t using_allocated_buckets = m_last.m_bucket_index - m_first.m_bucket_index + 1;
            if (to_add_a_reserve) { 
                result.new_m_buckets_capacity += (using_allocated_buckets / 2); 
            }

            result.new_m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, result.new_m_buckets_capacity);            
            T** new_first_allocated_bucket_ptr = result.new_m_buckets_ptr;
            if(to_add_a_reserve){
                new_first_allocated_bucket_ptr += (using_allocated_buckets / 2); 
            }

            T** new_last_allocated_bucket_ptr = new_first_allocated_bucket_ptr;
            
            size_t successful_allocated_buckets = 0;
            try{
                for (; successful_allocated_buckets < count_of_buckets; ++successful_allocated_buckets, ++new_last_allocated_bucket_ptr){
                    *new_last_allocated_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                }
            }
            catch(...){
                --new_last_allocated_bucket_ptr;
                while(successful_allocated_buckets > 0){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *new_last_allocated_bucket_ptr, BucketSize);
                    --new_last_allocated_bucket_ptr;
                    --successful_allocated_buckets;
                }
                throw;
            }

            T** old_buckets_pos = m_buckets_ptr + m_first_allocated_bucket_index;
            T** end_pos = m_buckets_ptr + m_last_allocated_bucket_index + 1;
            while (old_buckets_pos != end_pos){
                *new_last_allocated_bucket_ptr = *old_buckets_pos;
                ++new_last_allocated_bucket_ptr;
                ++old_buckets_pos;
            }
            --new_last_allocated_bucket_ptr;

            result.new_m_first_allocated_bucket_index = new_first_allocated_bucket_ptr - result.new_m_buckets_ptr;
            result.new_m_last_allocated_bucket_index = new_last_allocated_bucket_ptr - result.new_m_buckets_ptr;

            result.new_m_first.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_first.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_first.m_bucket_index = result.new_m_first_allocated_bucket_index + count_of_buckets + (m_first.m_bucket_index - m_first_allocated_bucket_index);
            result.new_m_first.m_cell_index = m_first.m_cell_index;
            result.new_m_first.m_ptr = m_first.m_ptr;       

            result.new_m_last.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_last.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_last.m_bucket_index = result.new_m_last_allocated_bucket_index - (m_last_allocated_bucket_index - m_last.m_bucket_index);
            result.new_m_last.m_cell_index = m_last.m_cell_index;
            result.new_m_last.m_ptr = m_last.m_ptr;
        }

        return result;
    }

    NewPtrsAndCapAfterRealloc realloc_with_add_allocated_buckets_to_the_end(size_t count_of_buckets, bool to_add_a_reserve){
        NewPtrsAndCapAfterRealloc result;
        if(!m_buckets_ptr){
            result.new_m_buckets_capacity = count_of_buckets;
            result.new_m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, result.new_m_buckets_capacity);
            
            T** new_first_allocated_bucket_ptr = result.new_m_buckets_ptr;
            T** new_last_allocated_bucket_ptr = new_first_allocated_bucket_ptr;
            
            try{
                for (size_t successful_allocated_buckets = 0; successful_allocated_buckets < count_of_buckets; ++successful_allocated_buckets, ++new_last_allocated_bucket_ptr){
                    *new_last_allocated_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                }
                --new_last_allocated_bucket_ptr;
            }
            catch(...){
                --new_last_allocated_bucket_ptr;
                T** end_pos = result.new_m_buckets_ptr - 1;
                while(new_last_allocated_bucket_ptr != end_pos){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *new_last_allocated_bucket_ptr, BucketSize);
                    --new_last_allocated_bucket_ptr;
                }
                throw;
            }

            result.new_m_first_allocated_bucket_index = new_first_allocated_bucket_ptr - result.new_m_buckets_ptr;
            result.new_m_last_allocated_bucket_index = new_last_allocated_bucket_ptr - result.new_m_buckets_ptr;

            result.new_m_first.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_first.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_first.m_bucket_index = result.new_m_first_allocated_bucket_index;
            result.new_m_first.m_cell_index = 0;
            result.new_m_first.m_ptr = result.new_m_buckets_ptr[result.new_m_first.m_bucket_index];

            result.new_m_last = result.new_m_first;
        }
        else{
            size_t old_count_of_allocated_buckets = m_last_allocated_bucket_index - m_first_allocated_bucket_index + 1;
            result.new_m_buckets_capacity = old_count_of_allocated_buckets + count_of_buckets;
            if (to_add_a_reserve) { 
                size_t using_allocated_buckets = m_last.m_bucket_index - m_first.m_bucket_index + 1;
                result.new_m_buckets_capacity += (using_allocated_buckets / 2); 
            }

            result.new_m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, result.new_m_buckets_capacity);            
            
            T** new_last_allocated_bucket_ptr = result.new_m_buckets_ptr + old_count_of_allocated_buckets;
            size_t successful_allocated_buckets = 0;
            try{
                for (; successful_allocated_buckets < count_of_buckets; ++successful_allocated_buckets, ++new_last_allocated_bucket_ptr){
                    *new_last_allocated_bucket_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                }
                --new_last_allocated_bucket_ptr;
            }
            catch(...){
                --new_last_allocated_bucket_ptr;
                while(successful_allocated_buckets > 0){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *new_last_allocated_bucket_ptr, BucketSize);
                    --new_last_allocated_bucket_ptr;
                    --successful_allocated_buckets;
                }
                throw;
            }

            T** old_buckets_pos = m_buckets_ptr + m_last_allocated_bucket_index;
            T** new_first_allocated_bucket_ptr = new_last_allocated_bucket_ptr - count_of_buckets; 
            T** end_pos = m_buckets_ptr + m_first_allocated_bucket_index - 1;
            while (old_buckets_pos != end_pos){
                *new_first_allocated_bucket_ptr = *old_buckets_pos;
                --new_first_allocated_bucket_ptr;
                --old_buckets_pos;
            }
            ++new_first_allocated_bucket_ptr;
                       
            result.new_m_first_allocated_bucket_index = new_first_allocated_bucket_ptr - result.new_m_buckets_ptr;
            result.new_m_last_allocated_bucket_index = new_last_allocated_bucket_ptr - result.new_m_buckets_ptr;

            result.new_m_first.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_first.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_first.m_bucket_index = result.new_m_first_allocated_bucket_index + (m_first.m_bucket_index - m_first_allocated_bucket_index);
            result.new_m_first.m_cell_index = m_first.m_cell_index;       
            result.new_m_first.m_ptr = m_first.m_ptr;       

            result.new_m_last.m_buckets_ptr = result.new_m_buckets_ptr;
            result.new_m_last.m_buckets_capacity = result.new_m_buckets_capacity;
            result.new_m_last.m_bucket_index = result.new_m_last_allocated_bucket_index - count_of_buckets - (m_last_allocated_bucket_index - m_last.m_bucket_index);
            result.new_m_last.m_cell_index = m_last.m_cell_index;
            result.new_m_last.m_ptr = m_last.m_ptr;
        }

        return result;
    }

public:

    explicit deque(): 
        m_buckets_ptr(nullptr), 
        m_first_allocated_bucket_index(0),
        m_last_allocated_bucket_index(0),
        m_first(m_buckets_ptr, 0, nullptr, 0 , 0), 
        m_last(m_buckets_ptr, 0, nullptr, 0, 0),
        m_size(0), 
        m_buckets_capacity(0), 
        m_alloc(Allocator()),
        m_alloc_ptr_on_bucket(m_alloc)
    {}

    explicit deque(const Allocator& alloc): 
        m_buckets_ptr(nullptr), 
        m_first_allocated_bucket_index(0),
        m_last_allocated_bucket_index(0),
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
    const_iterator begin() const {return {m_buckets_ptr, m_buckets_capacity, const_cast<const T*>(m_first.m_ptr), m_first.m_bucket_index, m_first.m_cell_index};}
    const_iterator cbegin() const noexcept {return begin();}

    //m_last pointing on the last element (not to the next position after last element, but straight at last element)
    iterator end() {
        if (m_buckets_ptr == nullptr){
            return {m_buckets_ptr, m_buckets_capacity, nullptr, 0, 0};
        }
        return {m_last + 1 };
    }
    const_iterator end() const {
        if (m_buckets_ptr == nullptr){
            return {m_buckets_ptr, m_buckets_capacity, nullptr, 0, 0};
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
            T** first_allocated_bucket_ptr = m_buckets_ptr + m_first_allocated_bucket_index;
            T** end_pos = m_buckets_ptr + (m_last_allocated_bucket_index + 1);
            while(first_allocated_bucket_ptr != end_pos){ 
                std::allocator_traits<Allocator>::deallocate(m_alloc, *first_allocated_bucket_ptr, BucketSize);
                ++first_allocated_bucket_ptr;
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, m_buckets_capacity);
            m_buckets_ptr = nullptr;
            m_first_allocated_bucket_index = 0; 
            m_last_allocated_bucket_index = 0;

            m_last.m_buckets_ptr = nullptr;
            m_last.m_buckets_capacity = 0;
            m_last.m_bucket_index = 0;
            m_last.m_ptr = nullptr;
            m_last.m_cell_index = 0;

            m_first = m_last;

            m_buckets_capacity = 0;
            return;
        }
       
        //deque is not empty:
        
        difference_type new_buckets_capacity = 
            (m_last.m_bucket_index - m_first.m_bucket_index)
            +
            ((m_last.m_cell_index < m_first.m_cell_index) ? 0 : 1);
        
        T** new_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket, new_buckets_capacity);
        decltype(new_buckets_capacity) success_allocated_count = 0;
        try{
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
        iterator new_deque_it(new_buckets_ptr, new_buckets_capacity, *new_buckets_ptr, 0, 0);
        iterator end_pos = end();
        try{
            while(current_deque_it != end_pos){
                std::allocator_traits<Allocator>::construct(m_alloc, new_deque_it.m_ptr, std::move_if_noexcept(*current_deque_it));
                ++current_deque_it;
                ++new_deque_it;
            }
            --new_deque_it;
        }
        catch(...){
            --new_deque_it;
            end_pos = --iterator(new_buckets_ptr, new_buckets_capacity, *new_buckets_ptr, 0, 0);
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
        
        T** first_allocated_bucket_ptr = m_buckets_ptr + m_first_allocated_bucket_index;
        T** end_bucket_pos = m_buckets_ptr + m_last_allocated_bucket_index + 1;
        while(first_allocated_bucket_ptr != end_bucket_pos){
            std::allocator_traits<Allocator>::deallocate(m_alloc, *first_allocated_bucket_ptr, BucketSize);
            ++first_allocated_bucket_ptr;
        }
        std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, m_buckets_capacity);
        
        m_buckets_ptr = new_buckets_ptr;
        m_buckets_capacity = new_buckets_capacity;

        m_first_allocated_bucket_index = first_allocated_bucket_ptr - new_buckets_ptr;
        m_last_allocated_bucket_index  = new_buckets_capacity - 1;

        m_first.m_buckets_ptr = new_buckets_ptr;
        m_first.m_buckets_capacity = new_buckets_capacity;
        m_first.m_bucket_index = m_first_allocated_bucket_index;
        m_first.m_cell_index = 0;
        m_first.m_ptr = *first_allocated_bucket_ptr;

        m_last = new_deque_it;
    }



    void clear() {
        erase(cbegin(), cend());
    }
    

    // iterator insert(const_iterator pos, const T& value){}
    
    // iterator insert(const_iterator pos, T&& value){}
    

    //iterator insert(const_iterator pos, size_type count, const T& value){}

    
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
            return {last.m_buckets_ptr, last.m_buckets_capacity, const_cast<T*>(last.m_ptr), last.m_bucket_index, last.m_cell_index};
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
            return {last.m_buckets_ptr, last.m_buckets_capacity, const_cast<T*>(last.m_ptr), last.m_bucket_index, last.m_cell_index}; 
        }
        else if (last == cend()){
            const_iterator end_pos = first - 1;
            while(m_last != end_pos){
                std::allocator_traits<Allocator>::destroy(m_alloc, m_last.m_ptr);
                --m_last;
                --m_size;
            }
            return {last.m_buckets_ptr, last.m_buckets_capacity, const_cast<T*>(last.m_ptr), last.m_bucket_index, last.m_cell_index};
        }

        else if (first.m_cell_index == 0 && last.m_cell_index == 0){

            //we move the delete bucket to the deque boundary to avoid unnecessary element movements
            //we check in which of halves is delete bucket to move him to the nearest border to reduce count of swaps
            difference_type count_delete_buckets = last.m_bucket_index - first.m_bucket_index;
            if ((m_last.m_bucket_index - last.m_bucket_index  + 1) <= (first.m_bucket_index - m_first.m_bucket_index)){
                //move to end

                difference_type old_distance_from_last_param_to_m_last = m_last.m_bucket_index - last.m_bucket_index;
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
                T** first_bucket_ptr = m_buckets_ptr + first.m_bucket_index;
                while (m_last.m_bucket_index - (first.m_bucket_index + count_delete_buckets - 1) >= count_delete_buckets){
                    for (difference_type i = count_delete_buckets - 1; i >= 0; --i){
                        std::swap(first_bucket_ptr[i], first_bucket_ptr[i+count_delete_buckets]);
                    }
                    first_bucket_ptr     += count_delete_buckets;
                    first.m_bucket_index += count_delete_buckets;
                }
                // now the distance between the last_bucket of the deck and the bucket block of pointers_to_delete_buckets is less than or equal the size of the bucket block
                // it means that we need to swap delete-block tail with reminder_end_buckets by 1 elemnt step.
                difference_type remainder_size = m_last.m_bucket_index - first.m_bucket_index + count_delete_buckets - 1;

                for (difference_type i = 0; i < remainder_size; ++i){
                    std::swap(first_bucket_ptr[i], first_bucket_ptr[i+count_delete_buckets]);
                }  
                first.m_bucket_index += remainder_size; // first.m_bucket_ptr is pointing on first trash_bucket
                first.m_ptr = m_buckets_ptr[first.m_bucket_index]; // updating m_ptr
                m_last.m_bucket_index = first.m_bucket_index - 1;

                decltype(first.m_bucket_index) end_index = first.m_bucket_index + count_delete_buckets;
                while(first.m_bucket_index != end_index){
                    std::allocator_traits<Allocator>::destroy(m_alloc, const_cast<T*>(first.m_ptr));
                    ++first; 
                }       

                return {m_buckets_ptr, m_buckets_capacity, const_cast<T*>(last.m_ptr), m_last.m_bucket_index - old_distance_from_last_param_to_m_last, last.m_cell_index};
            }

            // move to begin 
            T** first_bucket_ptr = m_buckets_ptr + first.m_bucket_index;
            while (first.m_bucket_index - m_first.m_bucket_index >= count_delete_buckets){
                for (difference_type i = count_delete_buckets - 1; i >= 0; --i){
                    std::swap(first_bucket_ptr[i], first_bucket_ptr[i - count_delete_buckets]);
                }
                first_bucket_ptr     -= count_delete_buckets;
                first.m_bucket_index -= count_delete_buckets;

            }
            difference_type reminder_size = first.m_bucket_index - m_first.m_bucket_index;

            for (difference_type i = 0; i < reminder_size; ++i){
                std::swap(first_bucket_ptr[-i + count_delete_buckets - 1], first_bucket_ptr[-i-1]);
            } 

            first.m_bucket_index -= reminder_size;
            m_first.m_bucket_index = first.m_bucket_index + count_delete_buckets; 

            while(first.m_bucket_index != m_first.m_bucket_index){
                std::allocator_traits<Allocator>::destroy(m_alloc, const_cast<T*>(first.m_ptr));
                ++first; 
            }        
            return {last.m_buckets_ptr, last.m_buckets_capacity, const_cast<T*>(last.m_ptr), last.m_bucket_index, last.m_cell_index};
        }

        //the worst case

        //Because, const_iterator::operator* returns const T& than we need to get a non const iterator to first (to avoid copy instead move)
        iterator first_it(first.m_buckets_ptr, first.m_buckets_capacity, const_cast<T*>(first.m_ptr), first.m_bucket_index, first.m_cell_index);
        iterator second_it(last.m_buckets_ptr, last.m_buckets_capacity, const_cast<T*>(last.m_ptr), last.m_bucket_index, last.m_cell_index);

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

        return {last.m_buckets_ptr, last.m_buckets_capacity, const_cast<T*>(last.m_ptr), last.m_bucket_index, last.m_cell_index};
    }


    void push_back( const T& value ){ 
        if (!m_buckets_ptr){
            auto result_of_realloc = realloc_with_add_allocated_buckets_to_the_end(1, true); 
             
            try{
                std::allocator_traits<Allocator>::construct(m_alloc, result_of_realloc.new_m_last.m_ptr, value);
            }
            catch(...){
                std::allocator_traits<Allocator>::deallocate(m_alloc, result_of_realloc.new_m_buckets_ptr[static_cast<size_t>(result_of_realloc.new_m_last_allocated_bucket_index)], BucketSize);
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, result_of_realloc.new_m_buckets_ptr, result_of_realloc.new_m_buckets_capacity);
                throw;
            }
            m_buckets_ptr = result_of_realloc.new_m_buckets_ptr;
            m_buckets_capacity = result_of_realloc.new_m_buckets_capacity;

            m_first_allocated_bucket_index = result_of_realloc.new_m_first_allocated_bucket_index;
            m_last_allocated_bucket_index  = result_of_realloc.new_m_last_allocated_bucket_index;


            m_first = result_of_realloc.new_m_first; 
            m_last = m_first;
            
            ++m_size;
        }
        else if (m_last.m_cell_index < static_cast<long int>(BucketSize - 1)){
            std::allocator_traits<Allocator>::construct(m_alloc, m_last.m_ptr + 1, value);
            if (m_size != 0) {
                ++m_last;
            }
        /*
            it is not appropriate to increment the entire iterator (++m_last) here, since the condition 
            satisfied guarantees that (++m_last) will not require a transition to the next bucket
        */
            ++m_size;
        }
        else {
            if (m_last.m_bucket_index + 1 < static_cast<long int>(m_buckets_capacity)){
                if(m_last.m_bucket_index != m_last_allocated_bucket_index){
                    std::allocator_traits<Allocator>::construct(m_alloc, m_buckets_ptr[m_last.m_bucket_index + 1], value);
                }
                else{
                    m_buckets_ptr[m_last_allocated_bucket_index + 1] = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                    try{
                        std::allocator_traits<Allocator>::construct(m_alloc, m_buckets_ptr[m_last_allocated_bucket_index + 1], value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(m_alloc, m_buckets_ptr[m_last_allocated_bucket_index + 1], BucketSize);
                        throw;
                    }
                    ++m_last_allocated_bucket_index;
                }
                ++m_last;
                ++m_size;
                return;
            }
            else{ // the worst case --> need reallocation
                auto result_of_realloc = realloc_with_add_allocated_buckets_to_the_end(1, true); 
                ++result_of_realloc.new_m_last;
                try{
                    std::allocator_traits<Allocator>::construct(m_alloc, result_of_realloc.new_m_last.m_ptr, value);
                }
                catch(...){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, result_of_realloc.new_m_buckets_ptr[static_cast<size_t>(result_of_realloc.new_m_last_allocated_bucket_index)], BucketSize);
                    std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, result_of_realloc.new_m_buckets_ptr, result_of_realloc.new_m_buckets_capacity);
                    throw;
                }
            

                m_first = result_of_realloc.new_m_first;
                m_last  = result_of_realloc.new_m_last;
                
                m_first_allocated_bucket_index = result_of_realloc.new_m_first_allocated_bucket_index;
                m_last_allocated_bucket_index  = result_of_realloc.new_m_last_allocated_bucket_index;
                
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


    void push_front(const T& value){
        if (!m_buckets_ptr){
            auto result_of_realloc = realloc_with_add_allocated_buckets_to_the_beginning(1, true); 
            // new_m_first & new_m_last points to the last cell in the only one bucket
            try{
                std::allocator_traits<Allocator>::construct(m_alloc, result_of_realloc.new_m_first.m_ptr, value);
            }
            catch(...){
                std::allocator_traits<Allocator>::deallocate(m_alloc, result_of_realloc.new_m_buckets_ptr[result_of_realloc.new_m_first_allocated_bucket_index], BucketSize);
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, result_of_realloc.new_m_buckets_ptr, result_of_realloc.new_m_buckets_capacity);
                throw;
            }
            m_buckets_ptr = result_of_realloc.new_m_buckets_ptr;
            m_buckets_capacity = result_of_realloc.new_m_buckets_capacity;

            m_first_allocated_bucket_index = result_of_realloc.new_m_first_allocated_bucket_index;
            m_last_allocated_bucket_index  = result_of_realloc.new_m_last_allocated_bucket_index;

            m_first = result_of_realloc.new_m_first; 
            m_last = m_first;
            
            ++m_size;
        }
        else if (m_first.m_cell_index != 0){
            --m_first;
            try{ 
                std::allocator_traits<Allocator>::construct(m_alloc, m_first.m_ptr, value);
            }
            catch(...){
                ++m_first;
                throw;
            }
            ++m_size;
        }
        else if (m_first.m_bucket_index != 0){
            bool is_allocated_new_bucket = false;
            if (m_first.m_bucket_index == m_first_allocated_bucket_index){
                m_buckets_ptr[m_first_allocated_bucket_index - 1] = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                is_allocated_new_bucket = true;
            }

            --m_first;
            try{ 
                std::allocator_traits<Allocator>::construct(m_alloc, m_first.m_ptr, value);
            }
            catch(...){
                if (is_allocated_new_bucket){    
                    std::allocator_traits<Allocator>::deallocate(m_alloc, m_buckets_ptr[m_first_allocated_bucket_index - 1], BucketSize);
                }
                ++m_first;
                throw;
            }

            if (m_size == 0){
                m_last = m_first;
            }
            ++m_size;
        }
        else { // the worst case --> we need to reallocation of outer array
            auto result_of_realloc = realloc_with_add_allocated_buckets_to_the_beginning(1, true);

            --result_of_realloc.new_m_first;
            try{ 
                std::allocator_traits<Allocator>::construct(m_alloc, result_of_realloc.new_m_first.m_ptr, value);
            }
            catch(...){
                std::allocator_traits<Allocator>::deallocate(m_alloc, result_of_realloc.new_m_buckets_ptr[result_of_realloc.new_m_first_allocated_bucket_index], BucketSize);
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, result_of_realloc.new_m_buckets_ptr, result_of_realloc.new_m_buckets_capacity);
                throw;
            }

            m_first = result_of_realloc.new_m_first;
            m_last  = result_of_realloc.new_m_last;
            
            m_first_allocated_bucket_index = result_of_realloc.new_m_first_allocated_bucket_index;
            m_last_allocated_bucket_index = result_of_realloc.new_m_last_allocated_bucket_index;
            
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket, m_buckets_ptr, m_buckets_capacity);
            m_buckets_ptr = result_of_realloc.new_m_buckets_ptr;
            m_buckets_capacity = result_of_realloc.new_m_buckets_capacity;
            ++m_size;
            return;
        }
        
        
        
    }

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
