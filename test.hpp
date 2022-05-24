// #include <iostream>
// #include <string>
// #include "InterfaceBlockCfg.hpp"

// // class Block {
// //     // private: 
// //     public:
// //         std::string name;
// //         int age;

// //         void setName( std::string str );
// //         std::string getName();
// //         void setAge( int i );
// //         int getAge();

// //         Block() {}
// //         ~Block() {}
// // };

// class Parrent : private Block
// {

//     public:

//         void setName( std::string str ) { error_log = str; }
//         std::string getName(  ) { return error_log; }

//         // void set_error_log( std::string s ) {}
//         // void set_error_page( std::string s ) {}
//         // void set_access_log( std::string s ) {}
//         // void set_client_max_body_size( std::string s ) {}

//         // std::string get_error_log() { return ""; }
//         // std::string get_error_page() { return ""; }
//         // std::string get_access_log() { return ""; }
//         // std::string get_client_max_body_size() { return ""; }


//         Parrent() {}
//         ~Parrent() {}
// };

// class Child : private Block {

//     private:
//         Child() { }

//     public:

//         // void set_error_log( std::string s ) {}
//         // void set_error_page( std::string s ) {}
//         // void set_access_log( std::string s ) {}
//         // void set_client_max_body_size( std::string s ) {}

//         // std::string get_error_log() { return ""; }
//         // std::string get_error_page() { return ""; }
//         // std::string get_access_log() { return ""; }
//         // std::string get_client_max_body_size() { return ""; }


//         void setName( std::string str ) { error_log = str; }
//         std::string getName(  ) { return error_log; }
        
//         Child(Parrent pr) {  }
//         ~Child() {}

// };
