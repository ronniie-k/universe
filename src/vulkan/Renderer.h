#include "util/Singleton.h"

class Renderer : public Singleton<Renderer>
{
public:
private:
	friend class Singleton<Renderer>;
};