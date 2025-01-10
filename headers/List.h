#ifndef LIST_PVM_2023
#define LIST_PVM_2023

#include <stdexcept>
#include <iostream>

template<typename T>
class List //Need T()
{
private:
    struct node
    {
        T data__;
        node* next__;
        node* prev__;
        node();
        ~node();
    };
    size_t size_;
    node* beg_;
    node* end_;
public:
    List(size_t size = 0)
    {
        size_ = 0;
        beg_ = nullptr;
        end_ = nullptr;
        if (size != 0)
        {
            for (size_t i = 0; i < size; ++i)
            {
                this->addElement(T());
            }
        }
    }
    size_t getSize() const { return size_; };
    ~List();
    void removeElement(size_t pos);
    T getElement(size_t  pos);
    void setElement(size_t pos, T value);
    const T& operator[](size_t pos)const;
    T& operator[](size_t pos);

    List(const List&);
    List(List&&);
    List& operator=(const List&);
    List& operator=(List&&);

    class iterator {
        node* current;
    public:
        iterator(node* el = nullptr) : current(el) {}
        iterator& operator++();
        iterator& operator--();
        iterator operator++(int);
        iterator& operator--(int);
        T& operator*() { return current->data__; }
        T& operator*() const { return current->data__; }
        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return current != other.current; }
        friend List;
    };
    iterator begin() const { return iterator(beg_); }
    iterator end() const { return iterator(nullptr); }
    void insert(const iterator& pos, const T& value);
    void remove(const iterator& pos);
    iterator addElement(const T& elem);
    iterator addElement(T&& elem);
};

//TODO
template<typename T>
void List<T>::insert(const iterator& pos, const T& value)
{
    node* new_node = new node;
    new_node->data__ = value;
    new_node->next__ = nullptr;
    new_node->prev__ = nullptr;

    if (pos.current == beg_) {
        // Insert at the begin of the node
        new_node->next__ = beg_;
        if(new_node->next__!= nullptr) new_node->next__->prev__ = new_node;
        beg_ = new_node;
        if (size_ == 0) end_ = new_node;
    }
    else {
        // Insert at the mid or end of the node
        node* previous = beg_;
        while (previous and previous->next__ && previous->next__ != pos.current) {
            previous = previous->next__;
        }

        new_node->next__ = previous->next__;
        previous->next__ = new_node;

        if (previous == end_) {
            end_ = new_node;
        }
    }

    ++size_;
}
template<typename T>
void List<T>::remove(const iterator& pos)
{
    if (beg_ == nullptr) {
        throw std::runtime_error("Cannot remove from an empty list");
    }

    if (pos.current == beg_) {
        // Remove the first node
        node* temp = beg_;
        beg_ = beg_->next__;
        if (beg_ != nullptr) {
            beg_->prev__ = nullptr;
        } else {
            end_ = nullptr; // List is now empty
        }
    } else {
        // Remove a middle or end node
        node* current = beg_;
        while (current != nullptr && current->next__ != pos.current) {
            current = current->next__;
        }

        if (current == nullptr || current->next__ == nullptr) {
            throw std::runtime_error("Iterator does not point to a valid node");
        }

        node* temp = current->next__;
        current->next__ = temp->next__;
        if (temp->next__ != nullptr) {
            temp->next__->prev__ = current;
        } else {
            end_ = current; // Removed the last node
        }
    }

    --size_;
}
template<typename T>
List<T>::List(const List& other){
    if (other.size_ == 0) {
        return;
    }

    size_ = 0;
    beg_ = nullptr;
    end_ = nullptr;
    node* current = other.beg_;
    while (current != nullptr) {
        addElement(current->data__);
        current = current->next__;
    }
}

template<typename T>
List<T>::List(List&& other){
    if (other.size_ == 0) {
        return;
    }

    size_ = 0;
    beg_ = nullptr;
    end_ = nullptr;
    node* current = other.beg_;
    while (current != nullptr) {
        addElement(std::move(current->data__));
        current = current->next__;
    }
}


template<typename T>
List<T>::~List() {
    delete beg_;
    beg_ = end_ = nullptr;
    size_ = 0;
}

template<typename T>
typename List<T>::iterator List<T>::addElement(const T& elem)
{
    node* Adder = new node;
    Adder->data__ = elem;
    Adder->next__ = nullptr;
    Adder->prev__ = nullptr;
    if (size_ != 0)
    {
        end_->next__ = Adder;
        Adder->prev__ = end_;
        end_ = Adder;
    }
    else
    {
        beg_ = Adder;
        end_ = Adder;
    }
    ++size_;
    return iterator(Adder);
}

template<typename T>
typename List<T>::iterator List<T>::addElement(T&& elem)
{
    node* Adder = new node;
    Adder->data__ = std::move(elem);
    Adder->next__ = nullptr;
    Adder->prev__ = nullptr;
    if (size_ != 0)
    {
        end_->next__ = Adder;
        Adder->prev__ = end_;
        end_ = Adder;
    }
    else
    {
        beg_ = Adder;
        end_ = Adder;
    }
    ++size_;
    return iterator(Adder);
}

template<typename T>
void List<T>::removeElement(size_t pos)
{
    node *now=beg_;
    size_t i=0;
    while(now!=nullptr)
    {
        if(i<pos)
        {
            now=now->next__;
            ++i;
        }
        else if(i==pos)
        {
            if(now->prev__==nullptr)
            {
                beg_=now->next__;
            }
            else
            {
                now->prev__->next__=now->next__;
            }
            if(now->next__==nullptr)
            {
                end_=now->prev__;
            }
            else
            {
                now->next__->prev__=now->prev__;
            }
            now->next__=nullptr;
            now->prev__=nullptr;
            delete now;
            --size_;
            return;
        }
    }
    throw std::out_of_range("Out of range");
} 

template<typename T>
List<T>& List<T>::operator=(const List& other) {
    delete beg_;
    beg_ = nullptr;
    end_ = nullptr;
    size_ = 0;

    if (other.size_ == 0) {
        return *this;
    }

    size_ = 0;
    beg_ = nullptr;
    end_ = nullptr;
    node* current = other.beg_;
    while (current != nullptr) {
        addElement(current->data__);
        current = current->next__;
    }
    return *this;
}

template<typename T>
List<T>& List<T>::operator=(List&& other) {
    delete beg_;
    beg_ = nullptr;
    end_ = nullptr;
    size_ = 0;

    if (other.size_ == 0) {
        return *this;
    }

    size_ = 0;
    beg_ = nullptr;
    end_ = nullptr;
    node* current = other.beg_;
    while (current != nullptr) {
        addElement(std::move(current->data__));
        current = current->next__;
    }
    return *this;
}

template<typename T>
void List<T>::setElement(size_t pos, T value) {
    // this func set element with value on pos 
    if (pos >= size_) {
        throw std::out_of_range("pos out of range");
    }

    node* current = beg_;
    for (size_t i = 0; i < pos; i++) {
        current = current->next__;
    }

    current->data__ = value;
}

template<typename T>
T List<T>::getElement(size_t pos)
{
    if (pos < 0 || pos >= size_) {
        throw std::out_of_range("wrong position");
    }

    node* current = beg_;

    for (size_t i = 0; i < pos; ++i) {
        current = current->next__;
    }

    return current->data__;
}

template<typename T>
List<T>::node::node(){
    next__=nullptr;
    prev__=nullptr;
    data__=T();
}

template<typename T>
List<T>::node::~node(){
    delete next__;
    //delete data__;
}

template<typename T>
typename List<T>::iterator& List<T>::iterator::operator++() {
    if (current) {
        current = current->next__;
    }
    else {
        throw "Iterator: out of bounds";
    }
    return *this;
}

template<typename T>
typename List<T>::iterator List<T>::iterator::operator++(int) {
    node* tmp = current;
    if (current) {
        current = current->next__;
    }
    else {
        throw "Iterator: out of bounds";
    }
    return iterator(tmp);
}

template<typename T>
typename List<T>::iterator& List<T>::iterator::operator--(){
    if (current) {
        current = current->prev__;
    }
    else {
        throw "Iterator: out of bounds";
    }
    return *this;
}

template<typename T>
typename List<T>::iterator& List<T>::iterator::operator--(int){
    node* tmp = current;
    if (current) {
        current = current->prev__;
    }
    else {
        throw "Iterator: out of bounds";
    }
    return iterator(tmp);
}

template<typename T>
const T& List<T>::operator[](size_t pos)const
{
    return getElement(pos);
}

template<typename T>
T& List<T>::operator[](size_t pos)
{
    return getElement(pos);
}

#endif // !LIST_PVM_2023
