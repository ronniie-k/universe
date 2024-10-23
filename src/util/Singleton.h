template<typename T>
class Singleton
{
public:
	static T &get()
	{
		static T ret;
		return ret;
	}

protected:
	Singleton() = default;
};