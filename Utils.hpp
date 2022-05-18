#ifndef Utils_hpp
#define Utils_hpp

#include <iostream>
#include <vector>
// #include <cctype>
// #include <algorithm>

#define RESET "\e[0m"
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define BLUE "\e[34m"
#define PURPLE "\e[35m"
#define CYAN "\e[36m"
#define GRAY "\e[37m"

// errors
// #define ERR_NOSUCHNICK 401				//"<nickname> :No such nick/channel"
// #define ERR_NOSUCHSERVER 402 			//" :No such server\n"		
// #define ERR_NOSUCHCHANNEL 403 			//":No such channel\n"
// #define ERR_CANNOTSENDTOCHAN 404		//"<channel name> :Cannot send to channel"  - Отсылается пользователю, который либо (a) не на канале,
// #define ERR_TOOMANYCHANNELS 405			//"<channel name> :You have joined too many channels"
// #define ERR_WASNOSUCHNICK 406
// #define ERR_TOOMANYTARGETS 407			//"<target> :Duplicate recipients. No message delivered"
// #define ERR_NOORIGIN 409				//":No origin specified"
// #define ERR_NORECIPIENT 411				//":No recipient given (<command>)"
// #define ERR_NOTEXTTOSEND 412			//":No text to send"
// #define ERR_NOTOPLEVEL 413				//"<mask> :No toplevel domain specified"
// #define ERR_WILDTOPLEVEL 414			//"<mask> :Wildcard in toplevel domain"
// #define ERR_NOMOTD 422 					//":MOTD File is missing"			
// #define ERR_NONICKNAMEGIVEN 431			//":No nickname given\n"	
// #define ERR_ERRONEUSNICKNAME 432 		//":Erroneus nickname\n"
// #define ERR_NICKNAMEINUSE 433 			//":Nickname is already in use\n"	
// #define ERR_NICKCOLLISION 436 			//"<nick> :Nickname collision KILL\n"
// #define ERR_NOTONCHANNEL 442			//":You're not on that channel\n"	
// #define ERR_USERONCHANNEL 443   		//"<user> <channel> :is already on channel"
// #define ERR_NOTREGISTERED 451			//":You have not registered\n"		
// #define ERR_NEEDMOREPARAMS 461			//":Not enough parameters\n"		
// #define ERR_ALREADYREGISTRED 462		//":You may not reregister\n"	
// #define ERR_PASSWDMISMATCH 464			//":Password incorrect\n"			
// #define ERR_KEYSET 467					//":Channel key already set\n"				
// #define ERR_CHANNELISFULL 471 			//"<channel> :Cannot join channel (+l)"
// #define ERR_UNKNOWNMODE 472				//"<char> :is unknown mode char to me"
// #define ERR_INVITEONLYCHAN 473			//"<channel> :Cannot join channel (+i)"
// #define ERR_BANNEDFROMCHAN 474 			//"<channel> :Cannot join channel (+b)"
// #define ERR_BADCHANNELKEY 475			//"<channel> :Cannot join channel (+k)"	
// #define ERR_NOPRIVILEGES 481			//":Permission Denied- You're not an IRC operator"
// #define ERR_CHANOPRIVSNEEDED 482		//":You're not channel operator\n"
// #define ERR_CANTKILLSERVER 483			//":You cant kill a server!"
// #define ERR_NOOPERHOST 491				//":No O-lines for your host\n"			
// #define ERR_UMODEUNKNOWNFLAG 501      	// ":Unknown MODE flag"
// #define ERR_USERSDONTMATCH 502			//":Cant change mode for other users\n"

// 		// replies
// #define RPL_UMODEIS 221 		//"<user mode string>"
// #define RPL_ADMINME 256 		//" :Admin name - "											
// #define RPL_ADMINLOC1 257		//"Location - Kazan, Republic of Tatarstan, Russian Federation\n"
// #define RPL_ADMINEMAIL 259		//":Admin email - "										
// #define RPL_AWAY 301			//"<nick> :<away message>"
// #define RPL_USERHOST 302		//":" [<reply>{<space><reply>}] <reply> ::= <nick>['*'] '=' <'+'|'-'><hostname>"
// #define RPL_ISON 303			//":"	
// #define RPL_UNAWAY 305			//":You are no longer marked as being away"
// #define RPL_NOWAWAY 306			//":You have been marked as being away"
// #define RPL_WHOISUSER 311		//"<nick> <user> <host> * :<real name>"
// #define RPL_WHOISSERVER 312		//"<nick> <server> :<server info>" 
// #define RPL_WHOISOPERATOR 313	//" :is an IRC operator\n"
// #define RPL_WHOWASUSER 314
// #define RPL_ENDOFWHO 315
// #define RPL_WHOISIDLE 317		//" <integer> :seconds idle\n"
// #define RPL_ENDOFWHOIS 318		//" :End of /WHOIS list\n"	
// #define RPL_WHOISCHANNELS 319
// #define RPL_LISTSTART 321		//"Channel :Users  Name\n"	
// #define RPL_LIST 322			//"<channel> <# visible> :<topic>"
// #define RPL_LISTEND 323         //":End of /LIST"
// #define	RPL_CHANNELMODEIS 324	//"<channel> <mode> <mode params>"
// #define RPL_NOTOPIC 331			//":No topic is set\n" 	
// #define RPL_TOPIC 332			//"<channel> :<topic>\n"			
// #define RPL_INVITING 341 		//"<channel> <nick>"
// #define RPL_VERSION 351			//" :RFC 1459  | May 1993\n"
// #define RPL_WHOREPLY 352		// ??
// #define RPL_NAMREPLY 353		//"<channel> :[[@|+]<nick> [[@|+]<nick> [...]]]" 
// #define RPL_ENDOFNAMES 366		//"<channel> :End of /NAMES list\n" 
// #define RPL_BANLIST 367			//"<channel> <banid>"                  
// #define RPL_ENDOFBANLIST 368	//"<channel> :End of channel ban list"
// #define RPL_ENDOFWHOWAS 369
// #define RPL_INFO 371			//":"									
// #define RPL_MOTD 372			//":- "									
// #define RPL_ENDOFINFO 374		//":End of /INFO list\n"			
// #define RPL_MOTDSTART 375		//" Message of the day - \n"		
// #define RPL_ENDOFMOTD 376		//":End of /MOTD command\n"			
// #define RPL_YOUREOPER 381		//":You are now an IRC operator\n"	
// #define RPL_REHASHING 382		//":Rehashing"						
// #define RPL_TIME 391			//" :Local time - "
// #define RPL_PING 700			       


#if __APPLE__
	#define IRC_NOSIGNAL SO_NOSIGPIPE
#else
	#define IRC_NOSIGNAL MSG_NOSIGNAL
#endif


std::vector<std::string> split(std::string str, std::string delimiter);
// std::string	itos( int const & num );
// bool	isAlphaStr(std::string str);
// bool	isDigitStr(std::string str);
// bool	isNumStr(std::string str);
// bool	contains( std::vector<std::string> vect, std::string str );
// bool	contains( std::vector<User*> vect, User * user );
// void	eraseString( std::vector<std::string> & vect, std::string name );
// void	eraseUser( std::vector<User*> & vect, User * user );
// int		checkWildcard(const char *str, const char *mask);
// bool	onlyWildcard(std::string str);

#endif 