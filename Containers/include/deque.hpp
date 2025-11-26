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
        T** bucket_ptr_;
        pointer ptr_;
        base_iterator(T** bucket_ptr, pointer ptr): bucket_ptr_(bucket_ptr), ptr_(ptr){}
    public:

        reference operator*() const {return *ptr_; }

	    pointer operator->() const {return ptr_;}

        base_iterator& operator++(){
            if (bucket_ptr_ != nullptr){
                if (const_cast<T*>(ptr_) - *bucket_ptr_ < static_cast<difference_type>(BucketSize-1)){
                   ++ptr_;
                }
                else{
                    ++bucket_ptr_;
                    ptr_ = *bucket_ptr_; 
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
            if (bucket_ptr_ != nullptr){
                if (ptr_ != *bucket_ptr_){
                    --ptr_;
                }
                else{
                    --bucket_ptr_;
                    ptr_ = *bucket_ptr_ + (BucketSize - 1);
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
            if (bucket_ptr_ != nullptr){
                difference_type result_index = (const_cast<T*>(ptr_) - *bucket_ptr_) + value % BucketSize;
                
                if (value >= static_cast<difference_type>(BucketSize))
                    bucket_ptr_ += value / BucketSize;
                
                if (result_index < static_cast<difference_type>(BucketSize)){
                    ptr_ = *bucket_ptr_ + result_index;
                }
                else{
                    ++bucket_ptr_;
                    ptr_ = *bucket_ptr_ + (result_index - BucketSize);
                } 
            }
            return *this;
        }

        base_iterator& operator-=(difference_type value) & {
            if (value < 0) {return *this += value;}
            if (bucket_ptr_ != nullptr){
                difference_type result_index_in_bucket = (const_cast<T*>(ptr_) - *bucket_ptr_) - value % BucketSize;
                
                if (value > static_cast<difference_type>(BucketSize))
                    bucket_ptr_ -= value / BucketSize;
            
                if (result_index_in_bucket > -1){
                    ptr_ = *bucket_ptr_ + result_index_in_bucket;
                } 
                else{
                    --bucket_ptr_;
                    ptr_ = *bucket_ptr_ + (BucketSize + result_index_in_bucket);
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
            if (bucket_ptr_ == nullptr || other.bucket_ptr_ == nullptr){
                return bucket_ptr_ - other.bucket_ptr_;
            }

            if(bucket_ptr_ == other.bucket_ptr_){
                return const_cast<T*>(ptr_) - const_cast<T*>(other.ptr_); 
            }
            else if(bucket_ptr_ > other.bucket_ptr_){
                return (
                    (((bucket_ptr_ - other.bucket_ptr_) -1) * BucketSize) 
                    + 
                    (const_cast<T*>(ptr_) - *bucket_ptr_) + ((*other.bucket_ptr_ + BucketSize) - const_cast<T*>(other.ptr_))
                );
            }
            else{
                return( 
                    (((other.bucket_ptr_ - bucket_ptr_) -1) * BucketSize)
                    + 
                    (const_cast<T*>(other.ptr_) - *other.bucket_ptr_) + ((*bucket_ptr_ + BucketSize) - const_cast<T*>(ptr_))
                ); 
            }
        }


        base_iterator& operator[](size_t index){return *(*this+index);}


        template<bool OtherIsConst>
        bool operator==(const base_iterator<OtherIsConst>& other){
            return bucket_ptr_ == other.bucket_ptr_ && ptr_ == other.ptr_; 
        }
        template<bool OtherIsConst>
        bool operator!=(const base_iterator<OtherIsConst>& other){
            return !(ptr_ == other.ptr_);
        }

        template<bool OtherIsConst>
        bool operator<(const base_iterator<OtherIsConst>& other){
            return bucket_ptr_ < other.bucket_ptr_ ? true : (bucket_ptr_ == other.bucket_ptr_ && ptr_ < other.ptr_) ? true : false; 
        }
        
        template<bool OtherIsConst>
        bool operator>(const base_iterator<OtherIsConst>& other){return other < *this;}

        template<bool OtherIsConst>
        bool operator>=(const base_iterator<OtherIsConst>& other){    return !(*this < other); }

        template<bool OtherIsConst>
        bool operator<=(const base_iterator<OtherIsConst>& other){    return !(*this > other); }

        operator base_iterator<true>(){return {bucket_ptr_, const_cast<const T*>(ptr_)};}
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
    T** buckets_ptr_; 
    T** first_allocated_bucket_ptr_;
    T** last_allocated_bucket_ptr_;
    base_iterator<false> first_;
    base_iterator<false> last_;
    size_type size_;
    size_type buckets_capacity_;
    Allocator alloc_;
    using AllocatorPtrOnBucket = typename std::allocator_traits<Allocator>::template rebind_alloc<T*>;
    AllocatorPtrOnBucket alloc_ptr_on_bucket_;


    void center_the_iterators_firs_and_last_(){
    /*
        Moving iterators (first_ and last_) to the begin of the middle allocated bucket of the deque,
        to optimize subsequent operations of inserting elements in the begin or end.
    */
        size_t index_of_middle_allocated_bucket = 
            (first_allocated_bucket_ptr_ - buckets_ptr_)
                +
            ((last_allocated_bucket_ptr_ - first_allocated_bucket_ptr_) / 2);

        last_.bucket_ptr_ = buckets_ptr_ + index_of_middle_allocated_bucket;
        last_.ptr_ = *last_.bucket_ptr_;
        first_ = last_ + 1;
    /*
        Now, if you insert an element at the end, the new element will be at position 
        last_+1, after which the iterator last_ is shuffled forward by 1 position. 
        Due to this, the iterators first_ and last_ point to position of new added element.                  
    */ 
    }


public:



    explicit deque(): 
        buckets_ptr_(nullptr), 
        first_allocated_bucket_ptr_(nullptr),
        last_allocated_bucket_ptr_(nullptr),
        first_(nullptr, nullptr), 
        last_(nullptr, nullptr),
        size_(0), 
        buckets_capacity_(0), 
        alloc_(Allocator()),
        alloc_ptr_on_bucket_(alloc_)
    {}

    explicit deque(const Allocator& alloc): 
        buckets_ptr_(nullptr), 
        first_allocated_bucket_ptr_(nullptr),
        last_allocated_bucket_ptr_(nullptr),
        first_(nullptr, nullptr), 
        last_(nullptr, nullptr), 
        size_(0), 
        buckets_capacity_(0), 
        alloc_(alloc),
        alloc_ptr_on_bucket_(alloc_)
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

    
    reference front() {return *first_;}
    const_reference front() const {return *first_;}

    reference back() {return *last_;}
    const_reference back() const {return *last_;}



    iterator begin() {return {first_};}
    const_iterator begin() const {return {first_.bucket_ptr_, const_cast<const T*>(first_.ptr_)};}
    const_iterator cbegin() const noexcept {return begin();}

    //last_ pointing on the last element (not to the next position after last element, but straight at last element)
    iterator end() {
        if (buckets_ptr_ == nullptr){
            return {nullptr, nullptr};
        }
        return {last_ + 1 };
    }
    const_iterator end() const {
        if (buckets_ptr_ == nullptr){
            return {nullptr, nullptr};
        }
        return {last_ + 1};
    } 
    const_iterator cend() const noexcept {return end();}


    reverse_iterator rbegin() {return std::make_reverse_iterator<iterator>(end());}
    const_reverse_iterator rbegin() const {return std::make_reverse_iterator<const_iterator>(cend());}
    const_reverse_iterator crbegin() const noexcept {return rbegin();}
    

    reverse_iterator rend() {return std::make_reverse_iterator<iterator>(begin());}
    const_reverse_iterator rend() const {return std::make_reverse_iterator<const_iterator>(cbegin());}
    const_reverse_iterator crend() const noexcept {return rend();}



    bool empty() const {return !size_;}
    
    size_type size() const {return size_;}
    
    long max_size() const {return std::numeric_limits<difference_type>::max();}

    void shrink_to_fit(){
        if (buckets_ptr_ == nullptr){ return; }
         
        if (size_ == 0){
            T** end_pos = last_allocated_bucket_ptr_ + 1;
            while(first_allocated_bucket_ptr_ != end_pos){ 
                std::allocator_traits<Allocator>::deallocate(alloc_, *first_allocated_bucket_ptr_, BucketSize);
                ++first_allocated_bucket_ptr_;
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, buckets_ptr_, buckets_capacity_);
            buckets_ptr_ = nullptr;
            first_allocated_bucket_ptr_ = last_allocated_bucket_ptr_ = nullptr;
            first_.bucket_ptr_= last_.bucket_ptr_ = nullptr;
            first_.ptr_ = last_.ptr_ = nullptr;
            buckets_capacity_ = 0;
            return;
        }
       
        //deque is not empty:
        
        difference_type new_buckets_capacity = 
            (last_.bucket_ptr_ - first_.bucket_ptr_)
            +
            (((last_.ptr_ - *last_.bucket_ptr_) < (first_.ptr_ - *first_.bucket_ptr_)) ? 0 : 1);
        
        T** new_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(alloc_ptr_on_bucket_, new_buckets_capacity);
        decltype(new_buckets_capacity) success_allocated_count = 0;
        try{
            for (; success_allocated_count < new_buckets_capacity; ++success_allocated_count){
                new_buckets_ptr[success_allocated_count] = std::allocator_traits<Allocator>::allocate(alloc_, BucketSize);
            }
        }
        catch(...){  
            for(decltype(success_allocated_count) i = 0; i < success_allocated_count; ++i){
                std::allocator_traits<Allocator>::deallocate(alloc_, new_buckets_ptr[i], BucketSize);
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, new_buckets_ptr, new_buckets_capacity);
            throw;
        }

        iterator current_deque_it = first_;
        iterator new_deque_it(new_buckets_ptr, *new_buckets_ptr);
        iterator end_pos = end();
        try{
            while(current_deque_it != end_pos){
                std::allocator_traits<Allocator>::construct(alloc_, new_deque_it.ptr_, std::move_if_noexcept(*current_deque_it));
            }
        }
        catch(...){
            --new_deque_it;
            end_pos = --iterator(new_buckets_ptr, *new_buckets_ptr);
            while (new_deque_it != end_pos){
                std::allocator_traits<Allocator>::destroy(alloc_, new_deque_it.ptr_);
                --new_deque_it;
            }
            for(decltype(success_allocated_count) i = 0; i < success_allocated_count; ++i){
                std::allocator_traits<Allocator>::deallocate(alloc_, new_buckets_ptr[i], BucketSize);
            }
            std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, new_buckets_ptr, new_buckets_capacity);
            throw;
        }

        clear();
        
        T** end_bucket_pos = last_allocated_bucket_ptr_ + 1;
        while(first_allocated_bucket_ptr_ != end_bucket_pos){
            std::allocator_traits<Allocator>::deallocate(alloc_, *first_allocated_bucket_ptr_, BucketSize);
        }
        std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, buckets_ptr_, buckets_capacity_);
        
        buckets_ptr_ = new_buckets_ptr;
        buckets_capacity_ = new_buckets_capacity;

        first_allocated_bucket_ptr_ = new_buckets_ptr;
        last_allocated_bucket_ptr_  = new_buckets_ptr + new_buckets_capacity - 1;

        first_.bucket_ptr_ = first_allocated_bucket_ptr_;
        first_.ptr_ = *first_allocated_bucket_ptr_;
        last_ = new_deque_it;
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
        if (size_ == 0) {return end();}

        if (first == last) {    
            return {last.bucket_ptr_, const_cast<T*>(last.ptr_)};
        } 
        /*
            like in gcc & clang here is no checking (first > last);
            it means that if (first > last) -> UB
        */
        if (first.ptr_ == first_.ptr_){
            if(last.ptr_ == cend().ptr_){ 
                while(first_ != last){
                    std::allocator_traits<Allocator>::destroy(alloc_, first_.ptr_);
                    ++first_;
                }

                // for that future inserts are inside the middle of the deck:
                first_.bucket_ptr_ = last_.bucket_ptr_ = buckets_ptr_+ (buckets_capacity_ / 2);
                first_.ptr_ = last_.ptr_ = *last_.bucket_ptr_;
                size_ = 0;
            }
            else{
                while(first_ != last){
                    std::allocator_traits<Allocator>::destroy(alloc_, first_.ptr_);
                    ++first_;
                    --size_;
                }
            } 
            return {last.bucket_ptr_, const_cast<T*>(last.ptr_)}; 
        }
        else if (last.ptr_ == cend().ptr_){
            const_iterator end_pos = first - 1;
            while(last_ != end_pos){
                std::allocator_traits<Allocator>::destroy(alloc_, last_.ptr_);
                --last_;
                --size_;
            }
            return {last.bucket_ptr_, const_cast<T*>(last.ptr_)};
        }

        else if (first.ptr_ == *first.bucket_ptr_ && last.ptr_ == *last.bucket_ptr_){

            //we move the delete bucket to the deque boundary to avoid unnecessary element movements
            //we check in which of halves is delete bucket to move him to the nearest border to reduce count of swaps
            difference_type count_delete_buckets = last.bucket_ptr_ - first.bucket_ptr_;
            if ((last_.bucket_ptr_ - last.bucket_ptr_  + 1) <= (first.bucket_ptr_ - first_.bucket_ptr_)){
                //move to end

                difference_type old_distance_from_last_to_end = last_.bucket_ptr_ - last.bucket_ptr_;
                /*
                difference_type old_distance_from_last_to_end:

                The point is that after the delete_buckets move is complete, the last.bucket_ptr_ pointer, which has type (T**),
                continues to point to its bucket_array position, but after moving the pointers on buckets, the last.bucket_ptr_ 
                pointer becomes invalid;
                So to keep the iterator to the real first non-deletable element, we keep the distance from its bucket to the 
                last bucket.
                IMPORTANT: the last.ptr_ pointer remains valid after all bucket moves, since we don't move the real buckets 
                themselves, but only the pointers to them.            
            */
                // As long as the loop condition is true, we can completely move the block of pointers_to_delete_buckets to the size of this block.
                while (last_.bucket_ptr_ - (first.bucket_ptr_ + count_delete_buckets - 1) >= count_delete_buckets){
                    for (difference_type i = count_delete_buckets - 1; i >= 0; --i){
                        std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i+count_delete_buckets]);
                    }
                    first.bucket_ptr_ +=count_delete_buckets; 
                }
                // now the distance between the last_bucket of the deck and the bucket block of pointers_to_delete_buckets is less than the size of the bucket block
                difference_type remainder_size = last_.bucket_ptr_ - first.bucket_ptr_ + count_delete_buckets - 1;

                for (difference_type i = 0; i < remainder_size; ++i){
                    std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i+count_delete_buckets]);
                }  
                first.bucket_ptr_ += remainder_size; // first.bucket_ptr_ is pointing on first trash_bucket
                first.ptr_ = *first.bucket_ptr_; // updating ptr_
                last_.bucket_ptr_ = first.bucket_ptr_ - 1;

                T** end_pos = first.bucket_ptr_ + count_delete_buckets;
                while(first.bucket_ptr_ != end_pos){
                    std::allocator_traits<Allocator>::destroy(alloc_, const_cast<T*>(first.ptr_));
                    ++first; 
                }       

                return {last_.bucket_ptr_ - old_distance_from_last_to_end, const_cast<T*>(last.ptr_)};
            }

            // move to begin
            while (first.bucket_ptr_ - first_.bucket_ptr_ >= count_delete_buckets){
                for (difference_type i = count_delete_buckets - 1; i >= 0; --i){
                    std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i - count_delete_buckets]);
                }
                first.bucket_ptr_ -= count_delete_buckets; 
            }
            difference_type reminder_size = first.bucket_ptr_ - first_.bucket_ptr_;

            for (difference_type i = 0; i < reminder_size; ++i){
                std::swap(first.bucket_ptr_[-i + count_delete_buckets - 1], first.bucket_ptr_[-i-1]);
            } 

            first.bucket_ptr_ -=  reminder_size; // first.bucket_ptr_ is pointing on first trach_bucket
            first_.bucket_ptr_ = first.bucket_ptr_ + count_delete_buckets; 

            while(first.bucket_ptr_ != first_.bucket_ptr_){
                std::allocator_traits<Allocator>::destroy(alloc_, const_cast<T*>(first.ptr_));
                ++first; 
            }        
            return {last.bucket_ptr_, const_cast<T*>(last.ptr_)};
        }

        //the worst case

        //Because, const_iterator::operator* returns const T& than we need to get a non const iterator to first (to avoid copy instead move)
        iterator first_it(first.bucket_ptr_, const_cast<T*>(first.ptr_));
        iterator second_it(last.bucket_ptr_, const_cast<T*>(last.ptr_));

        if (last_ - last < first - first_){
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
            while(last_ != first_it){
                std::allocator_traits<Allocator>::destroy(alloc_, last_.ptr_);
                --last_;
                --size_;
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
        while(first_ != second_it){
            std::allocator_traits<Allocator>::destroy(alloc_, first_.ptr_);
            ++first_;
            --size_;
        }

        return {last.bucket_ptr_, const_cast<T*>(last.ptr_)};
    }


    void push_back( const T& value ){ 
        if (!buckets_ptr_){
            buckets_ptr_ = std::allocator_traits<AllocatorPtrOnBucket>::allocate(alloc_ptr_on_bucket_, 1);
            try{
                *buckets_ptr_ = std::allocator_traits<Allocator>::allocate(alloc_, BucketSize);
                try{
                    std::allocator_traits<Allocator>::construct(alloc_, *buckets_ptr_, value);
                }
                catch(...){
                    std::allocator_traits<Allocator>::deallocate(alloc_, *buckets_ptr_, BucketSize);
                    throw;
                }
            }
            catch(...){
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, buckets_ptr_, 1);
                throw; 
            }
            
            first_allocated_bucket_ptr_ = last_allocated_bucket_ptr_ = buckets_ptr_;
            first_.bucket_ptr_ = last_.bucket_ptr_ = last_allocated_bucket_ptr_;
            first_.ptr_ = last_.ptr_ = *last_allocated_bucket_ptr_; 
            size_ = 1;
            buckets_capacity_ = 1;
        }
        else if ((last_.ptr_ - *last_.bucket_ptr_) < static_cast<long int>(BucketSize - 1)){
            std::allocator_traits<Allocator>::construct(alloc_, last_.ptr_ + 1, value);
            ++last_.ptr_;
        /*
            it is not appropriate to increment the entire iterator (++last_) here, since the condition 
            satisfied guarantees that (++last_) will not require a transition to the next bucket
        */
            ++size_;
        }
        else {
            if ((last_.bucket_ptr_ - buckets_ptr_) < static_cast<long int>(buckets_capacity_ - 1)){
                if(last_.bucket_ptr_ != last_allocated_bucket_ptr_){
                    std::allocator_traits<Allocator>::construct(alloc_, *(last_.bucket_ptr_ + 1), value);
                }
                else{
                    *(last_allocated_bucket_ptr_ + 1) = std::allocator_traits<Allocator>::allocate(alloc_, BucketSize);
                    try{
                        std::allocator_traits<Allocator>::construct(alloc_, *(last_allocated_bucket_ptr_ + 1), value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(alloc_, *(last_allocated_bucket_ptr_ + 1), BucketSize);
                        throw;
                    }
                    ++last_allocated_bucket_ptr_;
                }
                ++last_;
                ++size_;
                return;
            }
            else{ // the worst case – need reallocation
                size_t old_buckets_capacity = (last_allocated_bucket_ptr_ - first_allocated_bucket_ptr_ + 1);
                size_t new_buckets_capacity = old_buckets_capacity * 3;
                
                T** new_buckets_ptr = std::allocator_traits<AllocatorPtrOnBucket>::allocate(alloc_ptr_on_bucket_, new_buckets_capacity);

                T** old_buckets_pos = first_allocated_bucket_ptr_;
                T** new_buckets_pos = new_buckets_ptr + old_buckets_capacity;
            
                for (T** end_pos = last_allocated_bucket_ptr_ + 1; old_buckets_pos != end_pos; ++old_buckets_pos, ++new_buckets_pos){
                    *new_buckets_pos = *old_buckets_pos;
                }
                
                try{
                    *new_buckets_pos = std::allocator_traits<Allocator>::allocate(alloc_, BucketSize);
                    try{
                        std::allocator_traits<Allocator>::construct(alloc_, *new_buckets_pos, value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(alloc_, *new_buckets_pos, BucketSize);
                        throw;
                    }
                }
                catch(...){
                    std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, new_buckets_ptr, new_buckets_capacity);
                    throw;
                }

                first_.bucket_ptr_ = new_buckets_ptr + old_buckets_capacity + (first_.bucket_ptr_ - first_allocated_bucket_ptr_);
                last_.bucket_ptr_ = new_buckets_pos;
                last_.ptr_ = *last_.bucket_ptr_;
                
                first_allocated_bucket_ptr_ = new_buckets_ptr + old_buckets_capacity;
                last_allocated_bucket_ptr_ = new_buckets_pos;
                
                std::allocator_traits<AllocatorPtrOnBucket>::deallocate(alloc_ptr_on_bucket_, buckets_ptr_, buckets_capacity_);
                buckets_ptr_ = new_buckets_ptr;
                buckets_capacity_ = new_buckets_capacity;
                ++size_;
            }    
        }
    }


    
    //void push_back( T&& value ){}



    /*
    template< class... Args >
    reference emplace_back( Args&&... args );
    */


    void pop_back(){
        if (size_ == 0){return;}
        std::allocator_traits<Allocator>::destroy(alloc_, last_.ptr_);
        --last_;
        --size_;
        if (size_ == 0){
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
        if (size_ == 0){return;}
        std::allocator_traits<Allocator>::destroy(alloc_, first_.ptr_);
        ++first_;
        --size_;
        if (size_ == 0){
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
