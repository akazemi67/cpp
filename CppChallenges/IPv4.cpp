#include <iostream>
#include <array>
#include <string>
#include <sstream>

/**
 * This challenge is from the book: "The Modern C++ Challenge (2018)"
 * Here is the problem statement:
 *
 * Write a class that represents an IPv4 address. Implement the functions required to be able
 * to read and write such addresses from or to the console. The user should be able to input
 * values in dotted form, such as 127.0.0.1 or 168.192.0.100. This is also the form in
 * which IPv4 addresses should be formatted to an output stream.
 * Write a program that allows the user to input two IPv4 addresses representing a range and
 * list all the addresses in that range. Extend the structure defined to implement the requested functionality.
 */

class bad_ipv4_format : public std::exception {
public:
    [[nodiscard]] const char * what () const noexcept override {
        return "Invalid IPv4 format.";
    }
};

class IPv4{
private:
    std::array<int, 4> parts;
public:
    IPv4() : parts{0} {};
    explicit IPv4(unsigned long ipValue) : parts{static_cast<unsigned char>( (ipValue>>24) & 0xFF),
                                                 static_cast<unsigned char>( (ipValue>>16) & 0xFF),
                                                 static_cast<unsigned char>( (ipValue>>8) & 0xFF),
                                                 static_cast<unsigned char>( ipValue & 0xFF)} {}
    IPv4(int a, int b, int c, int d) : parts{a,b,c,d} {}
    IPv4(const IPv4 &ip) : parts(ip.parts) {}

    unsigned long to_ulong() const{
        return parts[3] | (parts[2]<<8) | (parts[1]<<16) | (parts[0]<<24);
    }
    IPv4& operator=(const IPv4 &ip){
        if(this == &ip)
            return *this;
        this->parts = ip.parts;
        return *this;
    }

    IPv4& operator++() noexcept{
        *this = IPv4(to_ulong()+1);;
        return *this;
    }
    IPv4 operator++(int) noexcept{
        IPv4 ip(*this);
        ++(*this);
        return ip;
    }

    IPv4& operator--() noexcept{
        *this = IPv4(to_ulong()-1);;
        return *this;
    }
    IPv4 operator--(int) noexcept{
        IPv4 ip(*this);
        --(*this);
        return ip;
    }

    bool operator==(const IPv4& ip) const noexcept{
        return to_ulong()==ip.to_ulong();
    }
    bool operator!=(const IPv4& ip) const noexcept{
        return !(*this==ip);
    }

    void clear();
    friend std::istream & operator>> (std::istream &in, IPv4 &ip);
    friend std::ostream & operator<< (std::ostream &out, const IPv4 &ip);
    friend bool operator<(const IPv4 &a, const IPv4 &b);
    friend bool operator>(const IPv4 &a, const IPv4 &b);
    friend bool operator<=(const IPv4 &a, const IPv4 &b);
    friend bool operator>=(const IPv4 &a, const IPv4 &b);
};

std::istream &operator>>(std::istream &in, IPv4 &ip) {
    std::string input, part;
    std::getline(in, input, '\n');
    std::stringstream stream(input);

    int i=0;
    bool ok = true;
    //reading ipv4 parts from input string and splitting them by dot
    while(ok && getline(stream, part, '.')){
        if(i<4) {
            ip.parts[i] = std::stoi(part);
            if(ip.parts[i]>255 || ip.parts[i]<0)
                ok = false;
        }else
            ok = false;
        i++;
    }

    //making sure we have read exactly 4 parts from input all of them between 0 and 255
    if(!ok){
        ip.clear();
        throw bad_ipv4_format();
    }
    return in;
}

std::ostream &operator<<(std::ostream &out, const IPv4 &ip) {
    const auto& n = ip.parts.size();
    for(auto i=0; i<n; i++){
        out << ip.parts[i];
        if(i!=n-1)
            out << ".";
    }
    return out;
}

void IPv4::clear() {
    for(auto &p : parts)
        p = 0;
}

bool operator<(const IPv4 &a, const IPv4 &b){
    return a.to_ulong() < b.to_ulong();
}

bool operator>(const IPv4 &a, const IPv4 &b) {
    return b < a;
}

bool operator<=(const IPv4 &a, const IPv4 &b) {
    return !(b < a);
}

bool operator>=(const IPv4 &a, const IPv4 &b) {
    return !(a < b);
}


int main(){
//    IPv4 ip{10,22,34,55}; //169222711
//    std::cout << "Initial Value: " << ip << " Dec: " << ip.to_ulong() << std::endl;
//    std::cout << "Inc1: " << ip++ << " Inc2: " << ++ip << std::endl;
//
//    IPv4 ip2 (169222711);
//    std::cout << "169222711 to ip: " << ip2 << std::endl;
//
//
//
//    std::cout << "Enter an ipv4: ";
//    std::cin >> ip;
//    std::cout << "You entered: " << ip << std::endl;
//    ip.clear();
//    std::cout << "After clear: " << ip << std::endl;

    IPv4 ip{10,20,30,40};
    IPv4 ip2{10,20,30,50};
    for(auto x=ip2; x>=ip; --x){
        std::cout << x << std::endl;
    }

    return 0;
}
