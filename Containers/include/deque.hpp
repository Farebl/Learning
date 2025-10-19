#ifndef FAREBL_DEQUE_H
#define FAREBL_DEQUE_H

//ДОРОБКА : коли ти робіш реалок зовнішнього масиву, роби перенос не від [first_; last_] а [first_allocated_; last_allocated];

#include <memory>
#include <limits>

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
            if (ptr_ - *bucket_ptr_ < static_cast<long int>(deque<T, Allocator>::bucket_size_-1)){
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
            
            if (value > static_cast<long int>(deque<T, Allocator>::bucket_size_))
                bucket_ptr_ += value / deque<T, Allocator>::bucket_size_;
            
            if (result_index < static_cast<long int>(deque<T, Allocator>::bucket_size_)){
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
            
            if (value > static_cast<difference_type>(deque<T, Allocator>::bucket_size_))
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
    T** first_allocated_bucket_ptr_;
    T** last_allocated_bucket_ptr_;
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



    explicit deque(): 
        buckets_ptr_(nullptr), 
        first_allocated_bucket_ptr_(nullptr),
        last_allocated_bucket_ptr_(nullptr),
        first_(nullptr, nullptr), 
        last_(nullptr, nullptr),
        size_(0), 
        buckets_capacity_(0), 
        alloc_(Allocator())
    {}

    explicit deque(const Allocator& alloc): 
        buckets_ptr_(nullptr), 
        first_allocated_bucket_ptr_(nullptr),
        last_allocated_bucket_ptr_(nullptr),
        first_(nullptr, nullptr), 
        last_(nullptr, nullptr), 
        size_(0), 
        buckets_capacity_(0), 
        alloc_(alloc){}
    
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
    const_iterator begin() const {return const_cast<std::remove_cv_t<decltype(this)>>(this)->begin();} //this is pointer on non-const object, that's why is call non-const begin() (to avoid recursive call)
    const_iterator cbegin() const noexcept {return begin();}

    //last_ pointing on the last element (not to the next position after last element, but straight at last element)
    iterator end() {return {last_.bucket_ptr_, last_.ptr_ + 1};}
    const_iterator end() const {return const_cast<std::remove_cv_t<decltype(this)>>(this)->end();} //this is pointer on non-const object, that's why is call non-const begin() (to avoid recursive call)
    const_iterator cend() const noexcept {return end();}


    reverse_iterator rbegin() {return std::make_reverse_iterator<iterator>(begin());}
    const_reverse_iterator rbegin() const {return const_cast<std::remove_cv_t<decltype(this)>>(this)->rbegin();}
    const_reverse_iterator crbegin() const noexcept {return std::make_reverse_iterator<const_iterator>(cbegin());}
    

    reverse_iterator rend() {return std::make_reverse_iterator<iterator>(end());}
    const_reverse_iterator rend() const {return const_cast<std::remove_cv_t<decltype(this)>>(this)->cend();}
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
            T** current_bucket_ptr = first_allocated_bucket_ptr_; // current_bucket_ptr pointing on empty buckets which need to deallocate
            while (current_bucket_ptr != first_.bucket_ptr_){
                std::allocator_traits<Allocator>::deallocate(alloc_, *current_bucket_ptr, bucket_size_);
                *current_bucket_ptr = nullptr;
                ++current_bucket_ptr;
            }
            current_bucket_ptr = last_.bucket_ptr_ + 1;
            T** next_pos_after_last_allocated_bucket = last_allocated_bucket_ptr_ + 1;
            while (current_bucket_ptr != next_pos_after_last_allocated_bucket){
                std::allocator_traits<Allocator>::deallocate(alloc_, *current_bucket_ptr, bucket_size_);
                *current_bucket_ptr = nullptr;
                ++current_bucket_ptr;
            }

            if (((*last_.bucket_ptr_ + bucket_size_ - 1) - last_.ptr_) < first_.ptr_ - *first_.bucket_ptr_){
            //the last basket has less empty space, so we move all the elements towards the end 
                iterator current_pos(last_.bucket_ptr_, *last_.bucket_ptr_ + bucket_size_-1);
                iterator new_last = current_pos;
                while(last_.ptr_ != first_.ptr_){
                    *current_pos = std::move(*last_);
                    --last_;
                    --current_pos;
                }
                *current_pos = std::move(*last_);
                first_ = current_pos;
                last_  = new_last;
            }
            else{   
                iterator current_pos(first_.bucket_ptr_, *first_.bucket_ptr_);
                iterator new_first = current_pos; 
                while(first_.ptr_ != last_.ptr_){
                    *current_pos = std::move(*first_);
                    ++first_;
                    ++current_pos;
                }
                *current_pos = std::move(*first_);
                first_ = new_first;
                last_  = current_pos;
            }

            ptrdiff_t new_capacity = last_.bucket_ptr_ - first_.bucket_ptr_ + 1;
            // now current_bucket_ptr pointing on the new allocated array;
            current_bucket_ptr = new T*[new_capacity]; // if new will throw exception -> memory will free automatically

            for (ptrdiff_t i = 0; i < new_capacity; ++i){
                current_bucket_ptr[i] = first_.bucket_ptr_[i];
            }

            delete buckets_ptr_;
            buckets_ptr_ = current_bucket_ptr;
            buckets_capacity_ = new_capacity;

            first_.bucket_ptr_ = current_bucket_ptr;
            last_.bucket_ptr_ = current_bucket_ptr+new_capacity - 1;

            first_allocated_bucket_ptr_ = first_.bucket_ptr_;
            last_allocated_bucket_ptr_ = last_.bucket_ptr_;
        }
    }



    void clear() {
        erase(cbegin(), cend());
    }
    

    // iterator insert(const_iterator pos, const T& value){}
    
    // iterator insert(const_iterator pos, T&& value){}
    

    iterator insert(const_iterator pos, size_type count, const T& value){
    /*
        !!!

        This emplementation provides a basic exception guarantee in case the
        T move-constructor will throw an exception

        !!!
    */

        if (count < 1) {return pos;} 

        if (buckets_ptr_ == nullptr){

            //This conditional branch provides the strong exception guarantee

            size_t new_buckets_capacity  = count % bucket_size_ == 0 ? count/bucket_size_ : count/bucket_size_+1;
            buckets_ptr_ = new T*[new_buckets_capacity]{}; // if new will throw exception -> delete[] will be called automatically
            size_t success_count = 0; // count of allocated buckets;
            try{
                for(; success_count < new_buckets_capacity ; ++success_count){
                    buckets_ptr_[success_count] = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                }
            }
            catch(...){
                for(size_t i = 0; i < success_count; ++i){
                    std::allocator_traits<Allocator>::deallocate(alloc_, buckets_ptr_[i], bucket_size_);
                }
                delete[] buckets_ptr_;
                buckets_ptr_ = nullptr;
                throw;
            }
            success_count = 0; // now, it is the count of success constructed elements;
            size_t i = 0, j = 0;
            try{
                for (; i<new_buckets_capacity ; ++i){
                    for(j = 0; j < bucket_size_ && success_count < count; ++j, ++success_count){
                        std::allocator_traits<Allocator>::construct(alloc_, buckets_ptr_[i] + j, value);
                    }
                }
            }
            catch(...){
                for(size_t i_ = 0; i_ <= i; ++i_){
                    for (size_t j_ = 0; j_ < j; ++j_){
                        std::allocator_traits<Allocator>::destroy(alloc_, buckets_ptr_[i] + j);
                    }
                } 
                
                for(size_t i = 0; i < success_count; ++i){
                    std::allocator_traits<Allocator>::deallocate(alloc_, buckets_ptr_[i], bucket_size_);
                }
                delete[] buckets_ptr_;
                buckets_ptr_ = nullptr;
                throw;
            }
            
            first_.bucket_ptr_ = buckets_ptr_;
            first_.ptr_ = *first_.bucket_ptr_;

            last_.bucket_ptr_ = buckets_ptr_[new_buckets_capacity -1];
            last_.ptr_ = *last_.bucket_ptr_ + j - 1;

            buckets_capacity_ = new_buckets_capacity ;
            size_ = count;
            return first_;
        } // end if (buckets_ptr_ == nullptr){
        else{
        /*
                Depending on different scenarios, there is 1 very important extreme case:
            Let pos be in the 4th quarter of the deque (closer to the end of the deque). 
            But there is not enough space at the end of the deque, that is we need to 
            allocate many new memory. At the same time, there is much more free space at 
            the beginning of the deque (that is we need to allocate much less new memory).
            
            And here there is a dilemma:
                1) move pos to the beginning by moving about 75% of the elements and at 
                the same time allocate new memory minimally,
                2) move pos to the end by moving about 25% of the elements but at the 
                same time allocate much more memory.
                
                That is, we choose between more moves with less memory allocation, and less
            moves with more memory allocation. Since this (for me) does not lend itself
            to rational analysis, I always choose the 2nd option:
                1) find the closest edge to pos;
                2) Move existing elements to the nearest edge, allocating memory as needed.
                (even if it takes a lot of memory)
        */

        /*
            Ok, we need to define in which side we need less count of elements moves.
                The side to which we need fewer element moves can be determined by comparing 
            the distance from the pos to the first element of the deque and half the number 
            of elements.
        */

        /*
            The following conditions are responsible for providing free allocated cells for new elements.
            Depending on some conditions, we will allocate new blocks or move already allocated blocks from the opposite side;
        */
            
            if (pos - last_ > size_/2){ //shift elements to end side
            //when we defined better side to move, we need to define are we having anought free allocated cells?   
                size_t count_of_free_cells_in_end = 
                    (last_allocated_bucket_ptr_ - last_.bucket_ptr_)*bucket_size_ + (*last_.bucket_ptr_ + bucket_size_ - last_.ptr_ - 1);
                
                if (count_of_free_cells_in_end < count){
                    // maybe, we have anogh nullptr-buckets in end and free whole allocated buckets on the opposite side?
                    size_t count_of_free_allocated_buckets_in_begin = first_.bucket_ptr_ - first_allocated_bucket_ptr_;
                    size_t count_of_nullptr_buckets_in_end = buckets_ptr_ + buckets_capacity_ - last_allocated_bucket_ptr_ - 1;
                    
                    if (
                        ((count_of_free_cells_in_end + (count_of_free_allocated_buckets_in_begin * bucket_size_)) > count) 
                        &&
                        count_of_nullptr_buckets_in_end >= count_of_free_allocated_buckets_in_begin
                    ) {
                    /*
                        in this case, we have anought free whole allocated buckets in begin
                        which we can swap with nullptr-buckets in and to avoid operator new() call;
                    */
                        for (size_t i = 0; i < count_of_nullptr_buckets_in_end; ++i){
                            std::swap(first_allocated_bucket_ptr_[i], last_allocated_bucket_ptr_[i + 1]);
                        }
                        first_allocated_bucket_ptr_+=count_of_nullptr_buckets_in_end;
                        last_allocated_bucket_ptr_+=count_of_nullptr_buckets_in_end;
                        // no return -> jumping to insertion new elements
                    }
                    else {
                        // In this case, we need to allocate additional memory
                        size_t count_of_new_needed_buckets = ((count - count_of_free_cells_in_end) % bucket_size_) == 0 ? 
                            (count - count_of_free_cells_in_end)/bucket_size_ : (count - count_of_free_cells_in_end)/bucket_size_ + 1;

                        if (count_of_nullptr_buckets_in_end < count_of_new_needed_buckets){
                        //It`s the worst case, because we need to reallocate outer array of ponters to buckets

                            size_t old_allocated_buckets_count = (last_allocated_bucket_ptr_ - first_allocated_bucket_ptr_ + 1);
                            size_t new_buckets_capacity = old_allocated_buckets_count * 3 + count_of_new_needed_buckets;
                            T** new_buckets = new T*[new_buckets_capacity]{}; // if new will throw exception -> delete[] will be called automatically

                            T** old_buckets_pos = first_allocated_bucket_ptr_;
                            T** new_buckets_pos = new_buckets + old_allocated_buckets_count; 
                        
                            for (;old_buckets_pos != last_allocated_bucket_ptr_; ++old_buckets_pos, ++new_buckets_pos){
                                *new_buckets_pos = *old_buckets_pos;
                            }
                            *new_buckets_pos = *old_buckets_pos; // old_buckets_pos == last_allocated_bucket_ptr_;
                            ++new_buckets_pos;

                            size_t count_of_success_allocated_new_buckets = 0;
                            try{
                                for (size_t i = 0; i < count_of_new_needed_buckets; ++i, ++count_of_success_allocated_new_buckets ){
                                    new_buckets_pos = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                                }  
                            }
                            catch(...){
                                --new_buckets_pos;
                                for(;count_of_success_allocated_new_buckets  > 0; --new_buckets_pos, --count_of_success_allocated_new_buckets ){
                                    std::allocator_traits<Allocator>::deallocate(new_buckets_pos, bucket_size_);
                                }
                                delete[] new_buckets;
                                throw;
                            }

                            /*
                                Now we have anought count of free cells, for new elements.
                                But, we have new allocated outer array of pointers to buckets, 
                                wich we need to free if T-copy constructor will throw an exception
                                
                                For that, we need to establish new last_ iterator
                            */

                            iterator new_last_it (last_.ptr_, new_buckets + old_allocated_buckets_count + (last_.bucket_ptr_ - first_allocated_bucket_ptr_)); 
                            iterator new_pos_it (pos.ptr_, new_buckets + old_allocated_buckets_count + (pos.bucket_ptr_ - first_allocated_bucket_ptr_));

                            size_t from_pos_to_end = last_ - pos + 1;
                            size_t successfully_constructed_count = 0;

                            iterator first_current_it = new_last_it + 1;
                            try{
                                for(
                                    size_t expected_success_count = count - from_pos_to_end; // count of new elements, which don't need moving - they are constructed right after old last_ position
                                    successfully_constructed_count < expected_success_count; 
                                    ++successfully_constructed_count, ++first_current_it
                                ){
                                    std::allocator_traits<Allocator>::construct(alloc_, first_current_it.ptr_, value); 
                                }
                            }
                            catch(...){
                                while(first_current_it != new_last_it){
                                    std::allocator_traits<Allocator>::destroy(alloc_, first_current_it.ptr_); 
                                    --first_current_it;
                                }
                                new_buckets_pos = old_allocated_buckets_count * 2 + count_of_success_allocated_new_buckets - 1;
                                for(;count_of_success_allocated_new_buckets  > 0; --new_buckets_pos, --count_of_success_allocated_new_buckets ){
                                    std::allocator_traits<Allocator>::deallocate(new_buckets_pos, bucket_size_);
                                }
                                delete[] new_buckets;
                                throw;
                            }

                            
                            first_current_it = last_;
                            iterator second_current_it = last_ + count;
                            successfully_constructed_count = 0; // now it means count of move-constructed of old elements between pos & end()
                            try{
                                for( ; first_current_it != pos; --first_current_it, --second_current_it, ++successfully_constructed_count){
                                    std::allocator_traits<Allocator>::construct(alloc_, second_current_it.ptr_, std::move(*first_current_it));
                                }
                                //first_current_it == pos; 
                                std::allocator_traits<Allocator>::construct(alloc_, second_current_it.ptr_, std::move(*first_current_it));
                                ++successfully_constructed_count;
                            }
                            catch(...){                                               
                                while (successfully_constructed_count > 0){
                                    ++second_current_it;
                                    std::allocator_traits<Allocator>::destroy(alloc_, second_current_it.ptr_); 
                                    --successfully_constructed_count;
                                }
                                new_buckets_pos = old_allocated_buckets_count * 2 + count_of_success_allocated_new_buckets - 1;
                                for(;count_of_success_allocated_new_buckets  > 0; --new_buckets_pos, --count_of_success_allocated_new_buckets ){
                                    std::allocator_traits<Allocator>::deallocate(new_buckets_pos, bucket_size_);
                                }
                                delete[] new_buckets;
                                throw;
                            }  


                            while(new_pos_it != last_){
                                *new_pos_it = value;
                            }
                            *new_pos_it = value; 

                            first_.bucket_ptr_ = new_buckets + old_allocated_buckets_count + (first_.bucket_ptr_ - first_allocated_bucket_ptr_);
                            last_ = second_current_it + successfully_constructed_count - 1; // successfully_constructed_count means count of move-constructed of old elements between pos & end()

                            first_allocated_bucket_ptr_ = new_buckets + old_allocated_buckets_count;
                            last_allocated_bucket_ptr_ = new_buckets_pos;

                            delete[] buckets_ptr_;
                            buckets_ptr_ = new_buckets;
                            buckets_capacity_ = new_buckets_capacity;

                            size_ += count;
                            return {new_pos_it + count};  
                        }
                        else {
                            // int this case we have anought nullptr pointers on buckets in end side
                            size_t count_of_success_allocate = 0;
                            try{    
                                for (size_t i = 0; i < count_of_new_needed_buckets; ++i, ++count_of_success_allocate){
                                    ++last_allocated_bucket_ptr_;
                                    std::allocator_traits<Allocator>::allocate(alloc_, last_allocated_bucket_ptr_, bucket_size_);
                                }
                            }
                            catch(...){
                                --last_allocated_bucket_ptr_;
                                for (; count_of_success_allocate > 0; --count_of_success_allocate){
                                    std::allocator_traits<Allocator>::deallocate(alloc_, last_allocated_bucket_ptr_, bucket_size_);
                                    *last_allocated_bucket_ptr_ = nullptr;
                                    --last_allocated_bucket_ptr_;
                                }
                                throw;
                            }
                            // no return -> jumping to insertion new elements
                        }
                    }                 
                } //end if (count_of_free_cells_in_end < count){...} 
                


                // now we have anought count of free cells, to insertion new elements

                size_t from_pos_to_end = last_ - pos + 1;
                size_t successfully_constructed_count = 0; 

                iterator first_current_it = last_+1;
                try{
                    for(
                        size_t expected_success_count = count - from_pos_to_end; // count of new elements, which don't need moving - they are constructed right after old last_ position
                        successfully_constructed_count < expected_success_count; 
                        ++successfully_constructed_count, ++first_current_it
                    ){
                        std::allocator_traits<Allocator>::construct(alloc_, first_current_it.ptr_, value); 
                    }
                }
                catch(...){
                    first_current_it = last_;
                    while (first_current_it != last_){
                        std::allocator_traits<Allocator>::destroy(alloc_, first_current_it.ptr_); 
                        --first_current_it;
                    }
                    throw;
                }


                first_current_it = last_;
                iterator second_current_it = last_ + count;
                successfully_constructed_count = 0;  // now it means count of move-constructed of old elements between pos & end()
                try{
                    for( ; first_current_it != pos; --first_current_it, --second_current_it, ++successfully_constructed_count){
                        std::allocator_traits<Allocator>::construct(alloc_, second_current_it.ptr_, std::move(*first_current_it));
                    }
                    //first_current_it == pos; 
                    std::allocator_traits<Allocator>::construct(alloc_, second_current_it.ptr_, std::move(*first_current_it));
                    ++successfully_constructed_count;
                }
                catch(...){                                               
                    while (successfully_constructed_count > 0){
                        ++second_current_it;
                        std::allocator_traits<Allocator>::destroy(alloc_, second_current_it.ptr_); 
                        --successfully_constructed_count;
                    }
                    throw;
                }  


  
                while(pos != last_){
                    *pos = value;
                    ++pos;
                }
                *pos = value; 

                last_ = second_current_it + successfully_constructed_count - 1; // successfully_constructed_count means count of move-constructed of old elements between pos & end()
                size_ += count;

                return pos - (count - 1);  
            }// end if (pos - last_ > size_/2){...}
            
            
/*
            ДИВИСЬ ТИ ВИНІС окрему ллогіку вствки елемнетів у випадорк реалокаціїї так як там треба працювати з новими 
            тимчасовими вказівниками new_last і new_pos які зав'язані на новий зовнішній масив. 
            У випадку кидання виключення, зовнішній масив звільняється

            Подумай над тим щоб винести логіку вставки або виідлення па'мяті в інші методи щоб у тебе не був один 
            момонілтний метод на 1000 рядків
*/
            else{ //shift elements to begin side
                size_t count_of_free_cells_in_begin = 
                    (first_.bucket_ptr_ - last_allocated_bucket_ptr_)*bucket_size_ + (first_.ptr_ - *first_.bucket_ptr_);
               
                if (count_of_free_cells_in_begin < count){
                    // maybe, we have anogh nullptr-buckets in end and free whole allocated buckets on the opposite side?
                    size_t count_of_free_allocated_buckets_in_end = last_allocated_bucket_ptr_ - last_.bucket_ptr_;
                    size_t count_of_nullptr_buckets_in_begin = first_allocated_bucket_ptr_ - buckets_ptr_;
                    
                    if (
                        ((count_of_free_cells_in_begin + (count_of_free_allocated_buckets_in_end * bucket_size_)) > count) 
                        &&
                        count_of_nullptr_buckets_in_begin >= count_of_free_allocated_buckets_in_end
                    ) {
                    /*
                        in this case, we have anought free whole allocated buckets in begin
                        which we can swap with nullptr-buckets in and to avoid operator new() call;
                    */
                        --first_allocated_bucket_ptr_; // move to nullptr cell of array of buckets_ptrs;
                        for (size_t i = 0; i < count_of_nullptr_buckets_in_begin; ++i, ++first_allocated_bucket_ptr_, --last_allocated_bucket_ptr_){
                            std::swap(*first_allocated_bucket_ptr_, last_allocated_bucket_ptr_[i]);
                        }
                        // no return -> jumping to insertion new elements
                    }
                    else {
                        // In this case, we need to allocate additional memory
                        size_t count_of_new_needed_buckets = ((count - count_of_free_cells_in_begin) % bucket_size_) == 0 ? 
                            (count - count_of_free_cells_in_begin)/bucket_size_ : (count - count_of_free_cells_in_begin)/bucket_size_ + 1;

                        if (count_of_nullptr_buckets_in_begin < count_of_new_needed_buckets){
                            // It`s the worst case, because we need to reallocate outer array of ponters to buckets
                            
                        }
                        else {
                            // in this case we have anought nullptr pointers on buckets in begin side
                            size_t count_of_success_allocate = 0;
                            try{    
                                for (size_t i = 0; i < count_of_new_needed_buckets; ++i, ++count_of_success_allocate){
                                    --first_allocated_bucket_ptr_;
                                    std::allocator_traits<Allocator>::allocate(alloc_, first_allocated_bucket_ptr_, bucket_size_);
                                }
                            }
                            catch(...){
                                ++first_allocated_bucket_ptr_;
                                for (; count_of_success_allocate > 0; --count_of_success_allocate){
                                    std::allocator_traits<Allocator>::deallocate(alloc_, first_allocated_bucket_ptr_, bucket_size_);
                                    *first_allocated_bucket_ptr_ = nullptr;
                                    ++first_allocated_bucket_ptr_;
                                }
                                throw;
                            }
                            // no return -> jumping to insertion new elements
                        }
                    }
                }

                // now we have anought count of free cells, to insertion new elements
                
                // some logic 

            } // end else{ //shift elements to begin side
       } // end else : if (buckets_ptr_ != nullptr){...}

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
        /*
        ДОАВЬ ОПТИМИЗАЦИЮ - когда ти удаляєш много елементов (> bucket_size)
        то у тебя возникают целие корзини которе модно удалить через перемщение вверхх.

        Но пере етим нужно обрезать края - переместить их на конец
            а потом пеермемещать целие контейнери
        протсо нужно поменять последние условие else (худший вариант) и віполанить его чатсично до предпосленего (где края контенера)
        */


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
                last_.ptr_ = first_.ptr_ = *first_.bucket_ptr_;
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
                
            //we move the delete bucket to the deque boundary to avoid unnecessary element movements
            //we check in which of halves is delete bucket to move him to the nearest border to reduce count of swaps
            ptrdiff_t count_delete_buckets = last.bucket_ptr_ - first.bucket_ptr_;
            if ((last_.bucket_ptr_ - first.bucket_ptr_ + count_delete_buckets - 1) <= (first.bucket_ptr_ - first_.bucket_ptr_)){
            //move to end

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
                (*last_.bucket_ptr_ + bucket_size_ - 1) frame variable the real-buckets with 
                count_of_deleting_buckets step.
            */

                ptrdiff_t remainder_size = last_.bucket_ptr_ - (first.bucket_ptr_+count_delete_buckets - 1);
                
                for (ptrdiff_t i = 0; i < remainder_size; ++i){
                    std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i+count_delete_buckets]);
                }  
                first.bucket_ptr_ += remainder_size; // first.bucket_ptr_ is pointing on first trash_bucket
                last_.bucket_ptr_ = first.bucket_ptr_-1;
                
                while(first.bucket_ptr_ != last.bucket_ptr_){
                    std::allocator_traits<Allocator>::destroy(alloc_, first.ptr_);
                    ++first; 
                }       

                iterator new_last(last_.bucket_ptr_ - old_distance_from_end_to_last, last.ptr_); 
                return new_last;
            }
            else{
                // move to begin
                while (first.bucket_ptr_ -first_.bucket_ptr_ >= count_delete_buckets){
                    for (ptrdiff_t i = count_delete_buckets-1; i >= 0; --i){
                        std::swap(first.bucket_ptr_[i], first.bucket_ptr_[i - count_delete_buckets]);
                    }
                    first.bucket_ptr_ -=count_delete_buckets; 
                }
                ptrdiff_t reminder_size = first.bucket_ptr_ - first_.bucket_ptr_;
                for (ptrdiff_t i = 0; i < reminder_size; ++i){
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
        // ДОРОБКА - new_capacity має бути не capacity*3 а (last_.bucket_ptr_ - first_.bucket_ptr_) * 3
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
            
            first_allocated_bucket_ptr_ = last_allocated_bucket_ptr_ = buckets_ptr_;

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
                if( last_.bucket_ptr_ == last_allocated_bucket_ptr_ ){
                    ++last_allocated_bucket_ptr_;
                    *last_allocated_bucket_ptr_ = std::allocator_traits<Allocator>::allocate(alloc_, bucket_size_);
                    try{
                        std::allocator_traits<Allocator>::construct(alloc_, *last_allocated_bucket_ptr_, value);
                    }
                    catch(...){
                        std::allocator_traits<Allocator>::deallocate(alloc_, *last_allocated_bucket_ptr_, bucket_size_);
                        *last_allocated_bucket_ptr_ = nullptr;
                        --last_allocated_bucket_ptr_;
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
                // old_buckets_pos == last_.bucket_ptr_;
                *new_buckets_pos = *old_buckets_pos;

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
                //
                first_.bucket_ptr_ = new_buckets_pos-(last_.bucket_ptr_ - first_.bucket_ptr_ + 1);
                last_.bucket_ptr_ = new_buckets_pos;
                last_.ptr_ = *last_.bucket_ptr_;

                first_allocated_bucket_ptr_ = first_.bucket_ptr_;
                last_allocated_bucket_ptr_ = last_.bucket_ptr_;
                
                delete[] buckets_ptr_;
                buckets_ptr_ = new_buckets;
                buckets_capacity_ = new_buckets_capacity;
                ++size_;
            }    
        }
    }


    //ОНОВИ ЦЕЙ МЕТОД
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
        else if ((last_.ptr_ - *last_.bucket_ptr_) < static_cast<long int>(bucket_size_-1)){
            std::allocator_traits<Allocator>::construct(alloc_, last_.ptr_+1, std::move(value));
            ++last_.ptr_;
        /*
            it is not appropriate to increment the entire iterator (++last_) here, since the condition 
            satisfied guarantees that (++last_) will not require a transition to the next bucket
        */
            ++size_;
        }
        else {
            if ((last_.bucket_ptr_ - buckets_ptr_) < static_cast<long int>(buckets_capacity_ - 1)){
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
