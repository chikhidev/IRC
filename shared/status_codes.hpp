#ifndef STATUS_CODES_HPP
#define STATUS_CODES_HPP

// --- Connection & Authentication ---
#define RPL_WELCOME         001 // "Welcome to the network"
#define ERR_NICKNAMEINUSE   433
#define ERR_NONICKNAMEGIVEN 431
#define ERR_ERRONEUSNICKNAME 432
#define ERR_ALREADYREGISTRED 462
#define ERR_PASSWDMISMATCH  464
#define ERR_NOTREGISTERED   451
#define ERR_ALREADYAUTH     462

// --- General Errors ---
#define ERR_UNKNOWNCOMMAND  421
#define ERR_NEEDMOREPARAMS  461
#define ERR_TOOMANYMATCHES  416
#define ERR_INPUTTOOLONG    414
#define ERR_NOTEXTTOSEND    412
#define ERR_NOORIGIN        409

// --- Channel Related ---
#define RPL_NOTOPIC         331
#define RPL_TOPIC           332
#define RPL_NAMREPLY        353
#define RPL_ENDOFNAMES      366
#define RPL_CHANNELMODEIS   324

#define ERR_NOSUCHCHANNEL   403
#define ERR_NOSUCHNICK      401
#define ERR_USERNOTINCHANNEL 441
#define ERR_NOTONCHANNEL    442
#define ERR_USERONCHANNEL   443
#define ERR_KEYSET          467
#define ERR_CHANNELISFULL   471
#define ERR_INVITEONLYCHAN  473
#define ERR_BADCHANNELKEY   475
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_BANNEDFROMCHAN  474

#define RPL_ENDNAMES        366

// --- INVITE ---
#define RPL_INVITING        341

// --- KICK ---
#define ERR_CANNOTSENDTOCHAN 404

// --- MODE ---
#define ERR_UNKNOWNMODE     472

// --- PING/PONG ---
#define RPL_NEUTRAL         0
#define ERR_NOORIGINSPECIFIED 409

// --- QUIT ---
#define RPL_QUIT            221 // you used this for goodbye

// --- WHOIS ---
#define RPL_WHOISUSER       311

#define ERR_UNKNOWNERROR    997

// --- Custom/Internal ---
#define STATUS_CLIENT_TIMEOUT      999
#define STATUS_PING_INTERVAL       998


#endif