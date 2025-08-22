#ifndef FAREBL_DEQUE_H
#define FAREBL_DEQUE_H

#include <memory>


namespace Farebl{

template <typename T, typename Allocator = std::allocator<T>>
class deque{
private:
    template <bool IsConst = false>
    class base_iterator{
    private:
        friend class deque<T, Allocator>;
        T** bucket_ptr_;
        T* ptr_;
        base_iterator(T** bucket_ptr, T* ptr): bucket_ptr_(bucket_ptr), ptr_(ptr){}
    public:
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = typename std::conditional<IsConst, const T*, T*>::type;
        using reference         = typename std::conditional<IsConst, const T&, T&>::type; 
        using iterator_category = std::random_access_iterator_tag;
    
        reference operator*() const {return *ptr_; }

	    pointer operator->() const {return ptr_;}

        base_iterator& operator++(){
            if (ptr_ - *bucket_ptr_ < deque<T, Allocator>::bucket_size_-1){
               ++ptr_;
            }
            else{
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
            if (ptr_ - *bucket_ptr_ > 0){
                --ptr_;
            }
            else{
                --bucket_ptr_;
                ptr_ = bucket_ptr_[deque<T, Allocator>::bucket_size_-1];
            }
            return *this;
        }

        base_iterator operator--(int){
            base_iterator temp = *this; 
            --(*this);
            return temp; 
        }
            

        base_iterator operator-(difference_type value){
            base_iterator temp = *this; 
            temp -= value;
            return temp; 
        }

        template<bool OtherIsConst>
        difference_type operator-(const base_iterator<OtherIsConst>& other){
            if(bucket_ptr_ == other.bucket_ptr_)
                return ptr_ - other.ptr_;
            
            else if(bucket_ptr_ > other.bucket_ptr_)
                return (
                    (((bucket_ptr_ - other.bucket_ptr_) -1) * deque<T, Allocator>::bucket_size_) 
                    + 
                    (ptr_ - *bucket_ptr_) + ((*other.bucket_ptr_ + deque<T, Allocator>::bucket_size_) - other.ptr_)
                );
            else 
                return( 
                    (((other.bucket_ptr_- bucket_ptr_) -1) * deque<T, Allocator>::bucket_size_)
                    + 
                    (other.ptr_ - *other.bucket_ptr_) + ((*bucket_ptr_ + deque<T, Allocator>::bucket_size_) - ptr_)
                ); 
                     
        }


        
        base_iterator& operator+=(difference_type value) & {
            if (value < 0) return *this -= value;

            difference_type result_index = (ptr_ - *bucket_ptr_) + value % deque<T, Allocator>::bucket_size_;
            
            if (value > deque<T, Allocator>::bucket_size_)
                bucket_ptr_ += value / deque<T, Allocator>::bucket_size_;
            
            if (result_index < deque<T, Allocator>::bucket_size_){
                ptr_ = *bucket_ptr_ + result_index;
            }
            else{
                ++bucket_ptr_;
                ptr_ = *bucket_ptr_ + (result_index - deque<T, Allocator>::bucket_size_);
            } 
            return *this;
        }

        base_iterator& operator-=(difference_type value) & {
            if (value < 0) {return *this += value;}
            
            difference_type result_index = (ptr_ - *bucket_ptr_) - value % deque<T, Allocator>::bucket_size_;
            
            if (value > deque<T, Allocator>::bucket_size_)
                bucket_ptr_ -= value / deque<T, Allocator>::bucket_size_;
        
            if (result_index > -1){
                ptr_ = *bucket_ptr_ + result_index;
            } 
            else{
                --bucket_ptr_;
                ptr_ = *bucket_ptr_ + (deque<T, Allocator>::bucket_size_+result_index );
            } 
            return *this;
        }


        base_iterator operator+(ptrdiff_t value) const {
            base_iterator temp = *(this);
            if (value<0)
                temp-=value;
            else
                temp+=value;

            return temp;
        }

        friend base_iterator operator+(ptrdiff_t value, const base_iterator& it) {
            base_iterator temp = it;
            if (value<0)
                temp-=value;
            else
                temp+=value;

            return temp; 
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

        operator base_iterator<true>(){return {bucket_ptr_, ptr_};}
    };

    T** buckets_ptr_; 
    base_iterator<false> first_;
    base_iterator<false> last_;
    size_t size_;
    size_t buckets_capacity_;
    Allocator alloc_;
    static const size_t bucket_size_ = (sizeof(T) < 256) ? 4096/sizeof(T) : 16; 


public: 
    using value_type             = T;
    using allocator_type         = Allocator;
    using pointer                = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer          = typename std::allocator_traits<Allocator>::const_pointer;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using size_type              = decltype(size_);
    using difference_type        = typename base_iterator<false>::difference_type;   

    using iterator               = base_iterator<false>;
    using const_iterator         = base_iterator<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;



    explicit deque(): buckets_ptr_(nullptr), first_(nullptr, nullptr), last_(nullptr, nullptr), size_(0), buckets_capacity_(0), alloc_(Allocator()){}

    explicit deque(const Allocator& alloc): buckets_ptr_(nullptr),  first_(nullptr, nullptr), last_(nullptr, nullptr), size_(0), buckets_capacity_(0), alloc_(alloc){}
    
    //explicit deque(size_type count, const Allocator& alloc) buckets_(nullptr), first_(nullptr), last_(nullptr), size_(0), alloc_(alloc){}


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
        delete[] buckets_ptr_;
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


    /* IMPORTANT for end/cend/rend:
    end() iterator pointing to the next address after the last_ element and belongs to the same bucket.
    that is end().bucket_ptr_ == last_.bucket_ptr_;
    This is necessary for the correct decrement work, because during --it or it-- there is a dereference 
    of the pointer to the bucket_ptr_;

    If, implementation of the end() was such as (last_ + 1), than the bucket_ptr_ of the result iterator of 
    the end() will pointing to the position, which is out_of_range of the outer array.
    That is, the end().bucket_ptr_ will pointing to buckets_ptr_[capacity] -> dereference such pointer is UB
    */

    iterator begin() {return {first_};}
    const_iterator begin() const {return begin();}
    const_iterator cbegin() const noexcept {return begin();}

    //last_ pointing on the last element (not to the next position after last element, but straight at last element)
    iterator end() {return {last_.bucket_ptr_, last_.ptr_ + 1};}
    const_iterator end() const {return end();}
    const_iterator cend() const noexcept {return end();}


    reverse_iterator rbegin() {return std::make_reverse_iterator<iterator>(begin());}
    const_reverse_iterator rbegin() const {return std::make_reverse_iterator<iterator>(cbegin());}
    const_reverse_iterator crbegin() const noexcept {return std::make_reverse_iterator<iterator>(cbegin());}
    

    reverse_iterator rend() {return std::make_reverse_iterator<iterator>(end());}
    const_reverse_iterator rend() const {return std::make_reverse_iterator<iterator>(cend());}
    const_reverse_iterator crend() const noexcept {return std::make_reverse_iterator<iterator>(cend());}



    bool empty() const {return !size_;}
    
    size_type size() const {return size_;}
    
    long max_size() const {return std::numeric_limits<difference_type>::max();}

    void shrink_to_fit(){
        if (size_ == 0){
            for(size_t i = 0; i < buckets_capacity_; ++i){ 
                std::allocator_traits<Allocator>::deallocate(alloc_, buckets_ptr_[i], bucket_size_);
            }
            delete[] buckets_ptr_;
            buckets_ptr_ = nullptr;
            buckets_capacity_ = 0;
        }
        else {
           
            T** current_bucket_ptr = buckets_ptr_; // current_bucket_ptr pointing on empty buckets which need to deallocate
            while (current_bucket_ptr != first_.bucket_ptr_){
                std::allocator_traits<Allocator>::deallocate(alloc_, *current_bucket_ptr, bucket_size_);
                *current_bucket_ptr = nullptr;
                ++current_bucket_ptr;
            }
            current_bucket_ptr = last_.bucket_ptr_ + 1;
            T** next_pos_after_last_current_bucket_ptr = buckets_ptr_ + buckets_capacity_;
            while (current_bucket_ptr != next_pos_after_last_current_bucket_ptr){
                std::allocator_traits<Allocator>::deallocate(alloc_, *current_bucket_ptr, bucket_size_);
                *current_bucket_ptr = nullptr;
                ++current_bucket_ptr;
            }
        
        
            if ((*last_.bucket_ptr_+bucket_size_-1) - last_.ptr_ < first_.ptr_ - first_.bucket_ptr_){
            //the last basket has less empty space, so we move all the elements towards the end 
                iterator current_pos(last_.bucket_ptr_, *last_.bucket_ptr_+bucket_size_-1);
                iterator new_last = current_pos;
                while(last_.ptr_ != first_.ptr_){
                    current_pos = std::move(*last_);
                    --last_;
                    --current_pos;
                }
                current_pos = std::move(*last_);
                first_ = current_pos;
                last_  = new_last;
            }
            else{   
                iterator current_pos(first_.bucket_ptr_, *first_.bucket_ptr_);
                iterator new_first = current_pos; 
                while(first_.ptr_ != last_.ptr_){
                    current_pos = std::move(*first_);
                    ++first_;
                    ++current_pos;
                }
                current_pos = std::move(*first_);
                first_ = new_first;
                last_  = current_pos;
            }

            ptrdiff_t new_capacity = last_.bucket_ptr_ - first_.bucket_ptr_ + 1;
            // now current_bucket_ptr pointing on the new allocated array;
            current_bucket_ptr = new T*[new_capacity]; // if new will throw exception -> memory will free automatically

            for (size_t i = 0; i < new_capacity; ++i){
                current_bucket_ptr[i] = first_.bucket_ptr_[i];
            }

            first_.bucket_ptr_ = current_bucket_ptr;
            last_.bucket_ptr_ = current_bucket_ptr+new_capacity - 1;
            delete buckets_ptr_;
            buckets_ptr_ = current_bucket_ptr;
            buckets_capacity_ = new_capacity;
        }
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
        if (!size_) {return end();}

        if (first == last) {    
            return {last.bucket_ptr_, last.ptr_};
        } 
        /*
            like in gcc & clang here is no checking (first > last);
            it means that if (first > last) -> UB
        */
        if (first.ptr_ == first_.ptr_){
            if(last.ptr_ == last_.ptr_+1){ // last.ptr_+1 -> end() -> is always the next address after last_.ptr_ even it is out of the bucket border
                while(first_ != last){
                    std::allocator_traits<Allocator>::destroy(alloc_, first_.ptr_);
                    ++first_;
                }
                last_.bucket_ptr_ = first_.bucket_ptr_ = buckets_ptr_+ (buckets_capacity_ / 2);
                last_.ptr = first_.ptr_ = *first_.bucket_ptr_;
                // that is for optimization for next operations of add elements
                size_ = 0;
            }
            else{
                while(first_ != last){
                    std::allocator_traits<Allocator>::destroy(alloc_, first_.ptr_);
                    ++first_;
                    --size_;
                }
            } 
            return {last.bucket_ptr_, last.ptr_};
        }
        else if (last.ptr_ == last_.ptr_+1){
            while(last_ != first){
                std::allocator_traits<Allocator>::destroy(alloc_, last_.ptr_);
                --last_;
                --size_;
            }
            //deleting first
            std::allocator_traits<Allocator>::destroy(alloc_, last_.ptr_);
            --last_;
            --size_;

            //this return was moved to the general body of the function to avoid the warning "non-void function does not return a value in all control paths"
            //return {last_.bucket_ptr_, last_.ptr_};
        }

        else if (first.ptr_ == *first.bucket_ptr_ && last.ptr_ == *last.bucket_ptr_){
                
            //we move the delete bucket to the queue boundary to avoid unnecessary element movements
            //we check in which of halves is delete bucket to move him to the nearest border to reduce count of swaps
            ptrdiff_t count_delete_buckets = last.bucket_ptr_ - first.bucket_ptr_;
            if ((last_.bucket_ptr_ - first.bucket_ptr_ + count_delete_buckets - 1) <= (first.bucket_ptr_ - first_.bucket_ptr_)){

            ptrdiff_t old_distance_from_end_to_last = last_.bucket_ptr_ - last.bucket_ptr_;
            /*
            ptrdiff_t old_distance_from_end_to_last:

                The point is that after the delete_buckets move is complete, the last.bucket_ptr_ pointer, which has type (T**),
                continues to point to its bucket_array position, but after moving the pointers on buckets, the last.bucket_ptr_ 
                pointer becomes invalid;
                So to keep the iterator to the real first non-deletable element, we keep the distance from its bucket to the 
                last bucket.
                IMPORTANT: the last.ptr_ pointer remains valid after all bucket moves, since we do not move the buckets 
                themselves, but only the pointers to them.            
            */
                while (last_.bucket_ptr_ - (first.bucket_ptr_+count_delete_buckets-1) >= count_delete_buckets){
                    for (ptrdiff_t i = count_delete_buckets-1; i >= 0; --i){
                        std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i+count_delete_buckets]);
                    }
                    first.bucket_ptr_ +=count_delete_buckets; 
                }
               /*
                    While the distance between the last trash-bucket and the real-last bucket more 
                or equal than count_of_deleting_buckets, we can swap the deleting buckets over 
*last_.bucket_ptr_+bucket_size_-1
frame variable
                the real-buckets with count_of_deleting_buckets step.
                */
                ptrdiff_t remainder_size = last_.bucket_ptr_ - (first.bucket_ptr_+count_delete_buckets - 1);
                
                for (size_t i = 0; i < remainder_size; ++i){
                    std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i+count_delete_buckets]);
                }  
                first.bucket_ptr_ += remainder_size; // first.bucket_ptr_ is pointing on first trach_bucket
                last_.bucket_ptr_ = first.bucket_ptr_-1;
                
                while(first.bucket_ptr_ != last.bucket_ptr_){
                    std::allocator_traits<Allocator>::destroy(alloc_, first.ptr_);
                    ++first; 
                }       

                iterator new_last(last_.bucket_ptr_ - old_distance_from_end_to_last, last.ptr_); 
                return new_last;
            }
            else{
                while (first.bucket_ptr_ -first_.bucket_ptr_ >= count_delete_buckets){
                    for (ptrdiff_t i = count_delete_buckets-1; i >= 0; --i){
                        std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i - count_delete_buckets]);
                    }
                    first.bucket_ptr_ -=count_delete_buckets; 
                }
                ptrdiff_t reminder_size = first.bucket_ptr_ - first_.bucket_ptr_;
                for (size_t i = 0; i < reminder_size; ++i){
                    std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i - count_delete_buckets]);
                }  
                first.bucket_ptr_ -=  reminder_size; // first.bucket_ptr_ is pointing on first trach_bucket
                first_.bucket_ptr_ = first.bucket_ptr_ - count_delete_buckets; 
            
                last = first_;
                while(first.bucket_ptr_ != last.bucket_ptr_){
                    std::allocator_traits<Allocator>::destroy(alloc_, first.ptr_);
                    ++first; 
                }        
                return {last.bucket_ptr_, last.ptr_};
            }

        }
        else{ //the worst case
            iterator first_it(first.bucket_ptr_, first.ptr_);
            //Because, const_iterator::operator* returns const T& than we get a non const iterator to first
           
            while(last.ptr_ != last_.ptr_){
                *first_it = std::move(*last); // if here will throw exception - nothing to repair;
                ++first_it;
                ++last;
            }
            *first_it = std::move(*last); // if here will throw exception - nothing to repair;

            last_ = first_it;
            // there, first points on the last real-element, and last poinst on last trash-element; 
            
            ++first_it; // there, first_it points on the first trash-element; 
            while(first_it.ptr_ != last.ptr_){
                std::allocator_traits<Allocator>::destroy(alloc_, first_it.ptr_);
                ++first_it;
                --size_;
            }
            std::allocator_traits<Allocator>::destroy(alloc_, first_it.ptr_);
            --size_;

            //this return was moved to the general body of the function to avoid the warning "non-void function does not return a value in all control paths"
            //return {last_.bucket_ptr_, last_.ptr_};
        }

        return {last_.bucket_ptr_, last_.ptr_};
    }


    void push_back( const T& value ){ 
        if (!buckets_ptr_){
            buckets_ptr_ = new T*[1]{}; // if new will throw exception -> memory will free automatically
            *buckets_ptr_ = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
            try{
                std::allocator_traits<Allocator>::construct(alloc_, *buckets_ptr_, value);
            }
            catch(...){
                std::allocator_traits<Allocator>::deallocate(alloc_, *buckets_ptr_, bucket_size_);
                delete[] buckets_ptr_;
                buckets_ptr_ = nullptr;
                throw;
            }
            last_.ptr_ = first_.ptr_ = *buckets_ptr_; // &buckets[0][0]
            last_.bucket_ptr_ = first_.bucket_ptr_ = buckets_ptr_; // &buckets_[0]
            size_ = 1;
            buckets_capacity_ = 1;
        }
        else if ((last_.ptr_ - *last_.bucket_ptr_) < (bucket_size_-1)){
            std::allocator_traits<Allocator>::construct(alloc_, last_.ptr_+1, value);
            ++last_.ptr_;
        /*
            it is not appropriate to increment the entire iterator (++last_) here, since the condition 
            satisfied guarantees that (++last_) will not require a transition to the next bucket
        */
            ++size_;
        }
        else {
            if ((last_.bucket_ptr_ - buckets_ptr_) < buckets_capacity_-1){
                if( !(*(last_.bucket_ptr_ + 1)) ){
                    *(last_.bucket_ptr_ + 1) = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                    try{
                        std::allocator_traits<Allocator>::construct(alloc_, *(last_.bucket_ptr_ + 1), value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(alloc_, *(last_.bucket_ptr_ + 1), bucket_size_);
                        *(last_.bucket_ptr_ + 1) = nullptr;
                        throw;
                    }
                }
                else{
                    std::allocator_traits<Allocator>::construct(alloc_, *(last_.bucket_ptr_ + 1), value);
                }
                ++last_.bucket_ptr_;
                last_.ptr_ = *last_.bucket_ptr_;
                ++size_;
            }
            else{
                size_t new_buckets_capacity = buckets_capacity_ == 1 ? 3 : buckets_capacity_ * 3;
                T** new_buckets = new T*[new_buckets_capacity]{}; // if new will throw exception -> delete[] will be called automatically

                T** old_buckets_pos = first_.bucket_ptr_;
                T** new_buckets_pos = new_buckets + (new_buckets_capacity - buckets_capacity_)/2 + (last_.bucket_ptr_ - first_.bucket_ptr_ + 1)/2;
            /*
                T** new_buckets_pos = &new_buckets[pos] where pos is the such that after pointers copy, the new_buckets[middle] 
                will be equal to the old_buckets[middle] that gives around equal free space in both sides of new_buckets; 
            */
                for (;old_buckets_pos != last_.bucket_ptr_; ++old_buckets_pos, ++new_buckets_pos){
                    *new_buckets_pos = *old_buckets_pos;
                }
                ++new_buckets_pos;

                *new_buckets_pos = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                try{
                    std::allocator_traits<Allocator>::construct(alloc_, *new_buckets_pos, value);
                }
                catch(...){
                    std::allocator_traits<Allocator>::deallocate(alloc_, *new_buckets_pos, bucket_size_);
                    delete[] new_buckets;
                    throw;
                }

                // last_.bucket_ptr_ and first_.bucket_ptr_ still pointing to the old backets_array
                first_.bucket_ptr_ = new_buckets_pos-(last_.bucket_ptr_ - first_.bucket_ptr_ + 1);
                last_.bucket_ptr_ = new_buckets_pos;
                last_.ptr_ = *last_.bucket_ptr_;
                
                delete[] buckets_ptr_;
                buckets_ptr_ = new_buckets;
                buckets_capacity_ = new_buckets_capacity;
                ++size_;
            }    
        }
    }

    void push_back( T&& value ){
        if (!buckets_ptr_){
            buckets_ptr_ = new T*[1]{}; // if new will throw exception -> memory will free automatically
            *buckets_ptr_ = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
            try{
                std::allocator_traits<Allocator>::construct(alloc_, *buckets_ptr_, std::move(value));
            }
            catch(...){
                std::allocator_traits<Allocator>::deallocate(alloc_, *buckets_ptr_, bucket_size_);
                delete[] buckets_ptr_;
                buckets_ptr_ = nullptr;
                throw;
            }
            last_.ptr_ = first_.ptr_ = *buckets_ptr_; // &buckets[0][0]
            last_.bucket_ptr_ = first_.bucket_ptr_ = buckets_ptr_; // &buckets_[0]
            size_ = 1;
            buckets_capacity_ = 1;
        }
        else if ((last_.ptr_ - *last_.bucket_ptr_) < (bucket_size_-1)){
            std::allocator_traits<Allocator>::construct(alloc_, last_.ptr_+1, std::move(value));
            ++last_.ptr_;
        /*
            it is not appropriate to increment the entire iterator (++last_) here, since the condition 
            satisfied guarantees that (++last_) will not require a transition to the next bucket
        */
            ++size_;
        }
        else {
            if ((last_.bucket_ptr_ - buckets_ptr_) < buckets_capacity_-1){
                if( !(*(last_.bucket_ptr_ + 1)) ){
                    *(last_.bucket_ptr_ + 1) = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                    try{
                        std::allocator_traits<Allocator>::construct(alloc_, *(last_.bucket_ptr_ + 1), std::move(value));
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(alloc_, *(last_.bucket_ptr_ + 1), bucket_size_);
                        *(last_.bucket_ptr_ + 1) = nullptr;
                        throw;
                    }
                }
                else{
                    std::allocator_traits<Allocator>::construct(alloc_, *(last_.bucket_ptr_ + 1), std::move(value));
                }
                ++last_.bucket_ptr_;
                last_.ptr_ = *last_.bucket_ptr_;
                ++size_;
            }
            else{
                size_t new_buckets_capacity = buckets_capacity_ == 1 ? 3 : buckets_capacity_ * 3;
                T** new_buckets = new T*[new_buckets_capacity]{}; // if new will throw exception -> delete[] will be called automatically

                T** old_buckets_pos = first_.bucket_ptr_;
                T** new_buckets_pos = new_buckets + (new_buckets_capacity - buckets_capacity_)/2 + (last_.bucket_ptr_ - first_.bucket_ptr_ + 1)/2;
            /*
                T** new_buckets_pos = &new_buckets[pos] where pos is the such that after pointers copy, the new_buckets[middle] 
                will be equal to the old_buckets[middle] that gives around equal free space in both sides of new_buckets; 
            */
                for (;old_buckets_pos != last_.bucket_ptr_; ++old_buckets_pos, ++new_buckets_pos){
                    *new_buckets_pos = *old_buckets_pos;
                }
                ++new_buckets_pos;

                *new_buckets_pos = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                try{
                    std::allocator_traits<Allocator>::construct(alloc_, *new_buckets_pos, std::move(value));
                }
                catch(...){
                    std::allocator_traits<Allocator>::deallocate(alloc_, *new_buckets_pos, bucket_size_);
                    delete[] new_buckets;
                    throw;
                }

                // last_.bucket_ptr_ and first_.bucket_ptr_ still pointing to the old backets_array
                first_.bucket_ptr_ = new_buckets_pos-(last_.bucket_ptr_ - first_.bucket_ptr_ + 1);
                last_.bucket_ptr_ = new_buckets_pos;
                last_.ptr_ = *last_.bucket_ptr_;
                
                delete[] buckets_ptr_;
                buckets_ptr_ = new_buckets;
                buckets_capacity_ = new_buckets_capacity;
                ++size_;
            }    
        }
    }



    /*
    template< class... Args >
    reference emplace_back( Args&&... args );
    */


    //void pop_back();


    //void push_front( const T& value );

    //void push_front( T&& value );


    /*
    template< class... Args >
    reference emplace_front( Args&&... args );
    */


    //void pop_front();
    

    //void resize( size_type count );

    //void resize( size_type count, const value_type& value );

    //void swap( deque& other ) noexcept(noexcept(std::allocator_traits<Allocator>::is_always_equal::value));

};




                                                                 
 






} // end namespace Farebl
#endif // FAREBL_DEQUE_H
