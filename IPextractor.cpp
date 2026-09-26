/*
Code author: AI (GPT 5.6 Sol Think in Copilot 9/26/2026)
Reviewer: Nickan Safi (9/26/2026)
*/
#include <cctype>
#include <iostream>
#include <string>

// Returns true if ch is allowed to be part of a candidate token.
bool isTokenCharacter(char ch)
/*
Code author: AI
Reviewer: Nickan Safi
*/
{
    unsigned char uch = static_cast<unsigned char>(ch);

    return std::isdigit(uch) || ch == '.' || ch == ':';
}

// Parses a number beginning at position pos.
//
// Requirements enforced:
//   - At least one digit
//   - No more than maxDigits digits
//   - Value cannot exceed maxValue
//   - No leading zero unless the number is exactly zero
//
// On success, pos is advanced past the digits and value receives
// the manually accumulated numeric value.
bool parseNumber(const std::string& str,
                 std::size_t& pos,
                 std::size_t end,
                 int maxDigits,
                 unsigned long maxValue,
                 unsigned long& value)
/*
Code author: AI
Reviewer: Nickan Safi
*/
{
    if (pos >= end ||
        !std::isdigit(static_cast<unsigned char>(str[pos])))
    {
        return false;
    }

    const std::size_t start = pos;
    int digitCount = 0;
    value = 0;

    while (pos < end &&
           std::isdigit(static_cast<unsigned char>(str[pos])))
    {
        ++digitCount;

        if (digitCount > maxDigits)
        {
            return false;
        }

        unsigned long digit =
            static_cast<unsigned long>(str[pos] - '0');

        value = value * 10UL + digit;

        if (value > maxValue)
        {
            return false;
        }

        ++pos;
    }

    // A multi-digit number cannot begin with zero.
    if (digitCount > 1 && str[start] == '0')
    {
        return false;
    }

    return true;
}

// Validates one complete candidate token in the half-open range
// [begin, end).
//
// The candidate must be exactly:
//
//   octet.octet.octet.octet
//
// or:
//
//   octet.octet.octet.octet:port
//
// No partial matches are accepted.
bool validateToken(const std::string& str,
                   std::size_t begin,
                   std::size_t end,
                   unsigned long& address,
                   int& port)
/*
Code author: AI
Reviewer: Nickan Safi
*/
{
    std::size_t pos = begin;
    unsigned long octets[4] = {0, 0, 0, 0};

    // Parse exactly four octets.
    for (int octetIndex = 0; octetIndex < 4; ++octetIndex)
    {
        if (!parseNumber(str,
                         pos,
                         end,
                         3,
                         255UL,
                         octets[octetIndex]))
        {
            return false;
        }

        // The first three octets must be followed by periods.
        if (octetIndex < 3)
        {
            if (pos >= end || str[pos] != '.')
            {
                return false;
            }

            ++pos;
        }
    }

    port = -1;

    // If characters remain, the only legal next character is
    // a colon beginning the optional port.
    if (pos < end)
    {
        if (str[pos] != ':')
        {
            return false;
        }

        ++pos;

        unsigned long portValue = 0;

        if (!parseNumber(str,
                         pos,
                         end,
                         5,
                         65535UL,
                         portValue))
        {
            return false;
        }

        port = static_cast<int>(portValue);
    }

    // The entire candidate must have been consumed.
    if (pos != end)
    {
        return false;
    }

    // Pack the octets into a 32-bit value in A.B.C.D order.
    address =
        (octets[0] << 24) |
        (octets[1] << 16) |
        (octets[2] << 8)  |
        octets[3];

    return true;
}

// Returns true if a valid address was found, false otherwise.
//
// On success:
//   outAddress holds the packed 32-bit address.
//   outPort holds the port, or -1 if no port was present.
//
// On failure:
//   outAddress is 0.
//   outPort is -1.
bool extractIPv4(const std::string& str,
                 unsigned long& outAddress,
                 int& outPort)
{
    // Establish the required failure-state values immediately.
    outAddress = 0;
    outPort = -1;

    std::size_t pos = 0;

    while (pos < str.size())
    {
        // Skip garbage characters.
        while (pos < str.size() &&
               !isTokenCharacter(str[pos]))
        {
            ++pos;
        }

        if (pos >= str.size())
        {
            break;
        }

        // Find the complete maximal candidate token.
        const std::size_t tokenBegin = pos;

        while (pos < str.size() &&
               isTokenCharacter(str[pos]))
        {
            ++pos;
        }

        const std::size_t tokenEnd = pos;

        unsigned long candidateAddress = 0;
        int candidatePort = -1;

        if (validateToken(str,
                          tokenBegin,
                          tokenEnd,
                          candidateAddress,
                          candidatePort))
        {
            outAddress = candidateAddress;
            outPort = candidatePort;
            return true;
        }

        // If validation failed, continue scanning after this complete
        // candidate. Do not look for a valid substring inside it.
    }

    return false;
}

int main()
/*
Code author: AI
Reviewer: Nickan Safi
*/
{
    std::string input;

    while (true)
    {
        std::cout << "Enter a string (or 'END' to quit): ";

        if (!std::getline(std::cin, input))
        {
            // Treat end-of-file like normal program termination.
            std::cout << "Program terminated." << '\n';
            break;
        }

        if (input == "END")
        {
            std::cout << "Program terminated." << '\n';
            break;
        }

        unsigned long address = 0;
        int port = -1;

        if (extractIPv4(input, address, port))
        {
            // Recover the individual octets from the packed value.
            unsigned long a = (address >> 24) & 0xFFUL;
            unsigned long b = (address >> 16) & 0xFFUL;
            unsigned long c = (address >> 8) & 0xFFUL;
            unsigned long d = address & 0xFFUL;

            std::cout
                << "Extracted IPv4 address: "
                << a << '.'
                << b << '.'
                << c << '.'
                << d
                << " (decimal value: "
                << address
                << ", port: ";

            if (port == -1)
            {
                std::cout << "none";
            }
            else
            {
                std::cout << port;
            }

            std::cout << ')' << '\n';
        }
        else
        {
            std::cout
                << "Invalid input: no valid IPv4 address found"
                << '\n';
        }
    }

    return 0;
}