#ifndef FAREBL_DEQUE_H
#define FAREBL_DEQUE_H

#include <memory>
#include <limits>

namespace Farebl{

template <typename T, typename Allocator = std::allocator<T>, size_t BucketSize = ((sizeof(T) < 256) ? 4096/sizeof(T) : 16)>
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
        friend class deque<T, Allocator, BucketSize>;
        T** m_bucket_ptr;
        pointer m_ptr;
        base_iterator(T** bucket_ptr, pointer ptr): m_bucket_ptr(bucket_ptr), m_ptr(ptr){}
    public:

        reference operator*() const {return *m_ptr; }

	    pointer operator->() const {return m_ptr;}

        base_iterator& operator++(){
            if (m_bucket_ptr != nullptr){
                if (const_cast<T*>(m_ptr) - *m_bucket_ptr < static_cast<difference_type>(BucketSize-1)){
                   ++m_ptr;
                }
                else{
                    ++m_bucket_ptr;
                    m_ptr = *m_bucket_ptr; 
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
            if (m_bucket_ptr != nullptr){
                if (m_ptr != *m_bucket_ptr){
                    --m_ptr;
                }
                else{
                    --m_bucket_ptr;
                    m_ptr = *m_bucket_ptr + (BucketSize - 1);
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
            if (m_bucket_ptr != nullptr){
                difference_type result_index = (const_cast<T*>(m_ptr) - *m_bucket_ptr) + value % BucketSize;
                
                if (value >= static_cast<difference_type>(BucketSize))
                    m_bucket_ptr += value / BucketSize;
                
                if (result_index < static_cast<difference_type>(BucketSize)){
                    m_ptr = *m_bucket_ptr + result_index;
                }
                else{
                    ++m_bucket_ptr;
                    m_ptr = *m_bucket_ptr + (result_index - BucketSize);
                } 
            }
            return *this;
        }

        base_iterator& operator-=(difference_type value) & {
            if (value < 0) {return *this += value;}
            if (m_bucket_ptr != nullptr){
                difference_type result_index_in_bucket = (const_cast<T*>(m_ptr) - *m_bucket_ptr) - value % BucketSize;
                
                if (value > static_cast<difference_type>(BucketSize))
                    m_bucket_ptr -= value / BucketSize;
            
                if (result_index_in_bucket > -1){
                    m_ptr = *m_bucket_ptr + result_index_in_bucket;
                } 
                else{
                    --m_bucket_ptr;
                    m_ptr = *m_bucket_ptr + (BucketSize + result_index_in_bucket);
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
            return m_bucket_ptr == other.m_bucket_ptr && m_ptr == other.m_ptr; 
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

        operator base_iterator<true>(){return {m_bucket_ptr, const_cast<const T*>(m_ptr)};}
    };
public: 
    using value_type             = T;
    using allocator_type         = Allocator;
    using pointer                = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer          = typename std::allocator_traits<Allocator>::const_pointer;
    using reference              = value_type&;
    using const_reference        = const value_type&;
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
    Allocator m_alloc;
    using AllocatorPtrOnBucket = typename std::allocator_traits<Allocator>::template rebind_alloc<T*>;
    AllocatorPtrOnBucket m_alloc_ptr_on_bucket_;


    void center_the_iterators_firs_and_last_(){
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
        m_first = m_last + 1;
    /*
        Now, if you insert an element at the end, the new element will be at position 
        last_+1, after which the iterator m_last is shuffled forward by 1 position. 
        Due to this, the iterators m_first and m_last point to position of new added element.                  
    */ 
    }


public:

    explicit deque(): 
        m_buckets_ptr(nullptr), 
        m_first_allocated_bucket_ptr(nullptr),
        m_last_allocated_bucket_ptr(nullptr),
        m_first(nullptr, nullptr), 
        m_last(nullptr, nullptr),
        m_size(0), 
        m_buckets_capacity(0), 
        m_alloc(Allocator()),
        m_alloc_ptr_on_bucket_(m_alloc)
    {}

    explicit deque(const Allocator& alloc): 
        m_buckets_ptr(nullptr), 
        m_first_allocated_bucket_ptr(nullptr),
        m_last_allocated_bucket_ptr(nullptr),
        m_first(nullptr, nullptr), 
        m_last(nullptr, nullptr), 
        m_size(0), 
        m_buckets_capacity(0), 
        m_alloc(alloc),
        m_alloc_ptr_on_bucket_(m_alloc)
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

     
    // reference operator[](size_type pos){}
    // const_reference operator[](size_type pos) const {}

    
    reference front() {return *m_first;}
    const_reference front() const {return *m_first;}

    reference back() {return *m_last;}
    const_reference back() const {return *m_last;}



    iterator begin() {return {m_first};}
    const_iterator begin() const {return {m_first.m_bucket_ptr, const_cast<const T*>(m_first.m_ptr)};}
    const_iterator cbegin() const noexcept {return begin();}

    //m_last pointing on the last element (not to the next position after last element, but straight at last element)
    iterator end() {
        if (m_buckets_ptr == nullptr){
            return {nullptr, nullptr};
        }
        return {m_last + 1 };
    }
    const_iterator end() const {
        if (m_buckets_ptr == nullptr){
            return {nullptr, nullptr};
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
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, m_buckets_ptr, m_buckets_capacity);
            m_buckets_ptr = nullptr;
            m_first_allocated_bucket_ptr = m_last_allocated_bucket_ptr = nullptr;
            m_first.m_bucket_ptr= m_last.m_bucket_ptr = nullptr;
            m_first.m_ptr = m_last.m_ptr = nullptr;
            m_buckets_capacity = 0;
            return;
        }
       
        //deque is not empty:
        
        difference_type new_buckets_capacity = 
            (m_last.m_bucket_ptr - m_first.m_bucket_ptr)
            +
            (((m_last.m_ptr - *m_last.m_bucket_ptr) < (m_first.m_ptr - *m_first.m_bucket_ptr)) ? 0 : 1);
        
        T** new_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket_, new_buckets_capacity);
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
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, new_buckets_ptr, new_buckets_capacity);
            throw;
        }

        iterator current_deque_it = m_first;
        iterator new_deque_it(new_buckets_ptr, *new_buckets_ptr);
        iterator end_pos = end();
        try{
            while(current_deque_it != end_pos){
                std::allocator_traits<Allocator>::construct(m_alloc, new_deque_it.m_ptr, std::move_if_noexcept(*current_deque_it));
            }
        }
        catch(...){
            --new_deque_it;
            end_pos = --iterator(new_buckets_ptr, *new_buckets_ptr);
            while (new_deque_it != end_pos){
                std::allocator_traits<Allocator>::destroy(m_alloc, new_deque_it.m_ptr);
                --new_deque_it;
            }
            for(decltype(success_allocated_count) i = 0; i < success_allocated_count; ++i){
                std::allocator_traits<Allocator>::deallocate(m_alloc, new_buckets_ptr[i], BucketSize);
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, new_buckets_ptr, new_buckets_capacity);
            throw;
        }

        clear();
        
        T** end_bucket_pos = m_last_allocated_bucket_ptr + 1;
        while(m_first_allocated_bucket_ptr != end_bucket_pos){
            std::allocator_traits<Allocator>::deallocate(m_alloc, *m_first_allocated_bucket_ptr, BucketSize);
        }
        std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, m_buckets_ptr, m_buckets_capacity);
        
        m_buckets_ptr = new_buckets_ptr;
        m_buckets_capacity = new_buckets_capacity;

        m_first_allocated_bucket_ptr = new_buckets_ptr;
        m_last_allocated_bucket_ptr  = new_buckets_ptr + new_buckets_capacity - 1;

        m_first.m_bucket_ptr = m_first_allocated_bucket_ptr;
        m_first.m_ptr = *m_first_allocated_bucket_ptr;
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
            return {last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
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
            }
            else{
                while(m_first != last){
                    std::allocator_traits<Allocator>::destroy(m_alloc, m_first.m_ptr);
                    ++m_first;
                    --m_size;
                }
            } 
            return {last.m_bucket_ptr, const_cast<T*>(last.m_ptr)}; 
        }
        else if (last == cend()){
            const_iterator end_pos = first - 1;
            while(m_last != end_pos){
                std::allocator_traits<Allocator>::destroy(m_alloc, m_last.m_ptr);
                --m_last;
                --m_size;
            }
            return {last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
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

                return {m_last.m_bucket_ptr - old_distance_from_last_to_end, const_cast<T*>(last.m_ptr)};
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
            return {last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
        }

        //the worst case

        //Because, const_iterator::operator* returns const T& than we need to get a non const iterator to first (to avoid copy instead move)
        iterator first_it(first.m_bucket_ptr, const_cast<T*>(first.m_ptr));
        iterator second_it(last.m_bucket_ptr, const_cast<T*>(last.m_ptr));

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

        return {last.m_bucket_ptr, const_cast<T*>(last.m_ptr)};
    }


    void push_back( const T& value ){ 
        if (!m_buckets_ptr){
            m_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket_, 1);
            try{
                *m_buckets_ptr = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                try{
                    std::allocator_traits<Allocator>::construct(m_alloc, *m_buckets_ptr, value);
                }
                catch(...){
                    std::allocator_traits<Allocator>::deallocate(m_alloc, *m_buckets_ptr, BucketSize);
                    throw;
                }
            }
            catch(...){
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, m_buckets_ptr, 1);
                throw; 
            }
            
            m_first_allocated_bucket_ptr = m_last_allocated_bucket_ptr = m_buckets_ptr;
            m_first.m_bucket_ptr = m_last.m_bucket_ptr = m_last_allocated_bucket_ptr;
            m_first.m_ptr = m_last.m_ptr = *m_last_allocated_bucket_ptr; 
            m_size = 1;
            m_buckets_capacity = 1;
        }
        else if ((m_last.m_ptr - *m_last.m_bucket_ptr) < static_cast<long int>(BucketSize - 1)){
            std::allocator_traits<Allocator>::construct(m_alloc, m_last.m_ptr + 1, value);
            ++m_last.m_ptr;
        /*
            it is not appropriate to increment the entire iterator (++m_last) here, since the condition 
            satisfied guarantees that (++m_last) will not require a transition to the next bucket
        */
            ++m_size;
        }
        else {
            if ((m_last.m_bucket_ptr - m_buckets_ptr) < static_cast<long int>(m_buckets_capacity - 1)){
                if(m_last.m_bucket_ptr != m_last_allocated_bucket_ptr){
                    std::allocator_traits<Allocator>::construct(m_alloc, *(m_last.m_bucket_ptr + 1), value);
                }
                else{
                    *(m_last_allocated_bucket_ptr + 1) = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                    try{
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
            else{ // the worst case – need reallocation
                size_t old_buckets_capacity = (m_last_allocated_bucket_ptr - m_first_allocated_bucket_ptr + 1);
                size_t new_buckets_capacity = old_buckets_capacity * 3;
                
                T** new_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(m_alloc_ptr_on_bucket_, new_buckets_capacity);

                T** old_buckets_pos = m_first_allocated_bucket_ptr;
                T** new_buckets_pos = new_buckets_ptr + old_buckets_capacity;
            
                for (T** end_pos = m_last_allocated_bucket_ptr + 1; old_buckets_pos != end_pos; ++old_buckets_pos, ++new_buckets_pos){
                    *new_buckets_pos = *old_buckets_pos;
                }
                
                try{
                    *new_buckets_pos = std::allocator_traits<Allocator>::allocate(m_alloc, BucketSize);
                    try{
                        std::allocator_traits<Allocator>::construct(m_alloc, *new_buckets_pos, value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(m_alloc, *new_buckets_pos, BucketSize);
                        throw;
                    }
                }
                catch(...){
                    std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, new_buckets_ptr, new_buckets_capacity);
                    throw;
                }

                m_first.m_bucket_ptr = new_buckets_ptr + old_buckets_capacity + (m_first.m_bucket_ptr - m_first_allocated_bucket_ptr);
                m_last.m_bucket_ptr = new_buckets_pos;
                m_last.m_ptr = *m_last.m_bucket_ptr;
                
                m_first_allocated_bucket_ptr = new_buckets_ptr + old_buckets_capacity;
                m_last_allocated_bucket_ptr = new_buckets_pos;
                
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(m_alloc_ptr_on_bucket_, m_buckets_ptr, m_buckets_capacity);
                m_buckets_ptr = new_buckets_ptr;
                m_buckets_capacity = new_buckets_capacity;
                ++m_size;
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
            center_the_iterators_firs_and_last_();
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
            center_the_iterators_firs_and_last_();
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
