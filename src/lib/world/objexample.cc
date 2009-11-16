#define WORLD_INSTANTIATE_STATIC_TEMPLATES
#include <world/world.h>
#include <iomanip>
using namespace std;
using namespace madness;

class Array : public WorldObject<Array> {
    vector<double> v;
public:
    /// Make block distributed array with size elements
    Array(World& world, size_t size)
            : WorldObject<Array>(world), v((size-1)/world.size()+1) {
        // do something ordered during construction
        process_pending();
    };

    /// Return the process in which element i resides
    ProcessID owner(size_t i) const {
        return i/v.size();
    };

    /// Read element i
    Future<double> read(size_t i) const {
        if (owner(i) == world.rank())
            return Future<double>(v[i-world.rank()*v.size()]);
        else
            return send(owner(i), &Array::read, i);
    };

    /// Write element i
    Void write(size_t i, double value) {
        if (owner(i) == world.rank())
            v[i-world.rank()*v.size()] = value;
        else
            send(owner(i), &Array::write, i, value);
        return None;
    };
};

int main(int argc, char** argv) {
    const int length = 10000;

    initialize(argc, argv);
    madness::World world(MPI::COMM_WORLD);
    Array a(world, length), b(world, length);

    if (world.rank() == 0) {
        std::cout << "data size " << length << "\n";
        std::cout << "World size is " << world.size() << "\n";
    }

    // Without regard to locality, initialize a[i]=i*10, b[i]=i*7
    for (int i=world.rank(); i<length; i+=world.size()) {
        a.write(i, 10.0*i);
        b.write(i,  7.0*i);
    }

    if (world.rank()==0)
        std::cout << "write section ends\n";
    world.gop.fence();// wait until all message passing end


    // All processes verify 10 random values from each array
    for (int j=0; j<10; j++) {
        size_t i = world.rand()%length;
        std::cout << "rank " << world.rank() << ": " << std::setw(4) << i <<
            "th data is owned by " << a.owner(i) << "\n";

        Future<double> vala = a.read(i);
        Future<double> valb = b.read(i);

        // Could do work here until results are available

        MADNESS_ASSERT(vala.get() == 10.0*i);
        MADNESS_ASSERT(valb.get() ==  7.0*i);

    }
    world.gop.fence();

    if (world.rank() == 0) print("OK!");
    finalize();
}
