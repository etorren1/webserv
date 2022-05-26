#ifndef ERRORS_HPP
# define ERRORS_HPP

# include <iostream>
# include <map>

class Errors {
private:
    std::map<int, std::string> errMap;
public:
    Errors();
    ~Errors();

    void    findErrorCode(std::string, int);
    void    return_error_page(std::string );
};

#endif