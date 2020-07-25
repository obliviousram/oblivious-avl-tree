#ifndef _IDGENERATOR_H
#define _IDGENERATOR_H

class IdGenerator
{
private:
    int id = 0;
public:
    IdGenerator() { };
    int getNextId() { return id++; };
    ~IdGenerator() { };
};
#endif