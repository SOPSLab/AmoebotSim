#include "system.h"

System::System()
{
}

System::System(const System* other) :
    particles(other->particles)
{
}
