KBase_Demo
==========

some imitations of a series of remarkable infrastructures.

some will be re-written with C++11 features; and the most will be modified to adapt to my personal needs.

### AtExitManager

provide a facility similar to the CRT `atexit()`. actually is a imitation of `base::AtExitManager`

the usage is simple. early in the `main()` or `WinMain()` scope create an object on the stack:

```c++
int main()
{
    KBase::AtExitManager exitManager;
}
```

when the object goes out of scope, all registered callbacks and the singleton destructors will be called

### Pickle

a light-weight serialization/deserialization facility.

support built-in types, and `std::string` and `std::wstring`.

an illustration of a `Pickle` object memory layout

```
<-----------capacity------------>
+------+----+----+---+----+-----+
|header|seg1|seg2|...|segn|     |
+------+----+----+---+----+-----+
       <-----payload------>
```

an illumination for basic functionalities

```c++
KBase::Pickle pickle;
// serialize
pickle.WriteInt(0x1234);
pickle.WriteBool(true);
pickle.WriteDouble(3.14159);
pickle.WriteString("hell world");

std::cout << "marshal data complete" << std::endl;

// transmit serialized data if necessary
size_t buffer_size = pickle.size();
std::unique_ptr<char[]> buff(new char[buffer_size]);
memcpy(buff.get(), pickle.data(), buffer_size);

KBase::Pickle pickle_out(buff.get(), buffer_size);

int protocol = 0;
bool flag = false;
double pi = 0.0;
std::string str;

// deserialize
KBase::PickleIterator it(pickle_out);
it.ReadInt(&protocol);
it.ReadBool(&flag);
it.ReadDouble(&pi);
it.ReadString(&str);
```

for complete functions and their usage, see the source code.