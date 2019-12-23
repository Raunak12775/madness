/*
 * test_macrotask.cc
 *
 *  Created on: Dec 17, 2019
 *      Author: fbischoff
 */
#define WORLD_INSTANTIATE_STATIC_TEMPLATES

#include <iostream>
#include <madness/world/MADworld.h>
#include <madness/world/worlddc.h>
#include <random>
#include <madness/mra/mra.h>
#include <madness/mra/funcimpl.h>

using namespace madness;



/// for each process create a world using a communicator shared with other processes by round-robin
/// copy-paste from test_world.cc
std::shared_ptr<World> create_worlds(World& universe, const std::size_t nworld) {

	if (universe.size()<nworld) {
		print("trying to create ",nworld,"world with",universe.size(), "processes");
		MADNESS_EXCEPTION("increase number of processes",1);
	}

	if (universe.rank()==0) print("== multiple worlds created with Intracomm::Create()==",nworld);
    std::vector<std::vector<int> > process_list(nworld);
    std::shared_ptr<World> all_worlds;

	for (int i=0; i<universe.size(); ++i) process_list[i%nworld].push_back(i);
	if (universe.rank()<nworld) print("process_list",process_list[universe.rank()]);


	for (int i=0; i<process_list.size(); ++i) {
		const std::vector<int>& pl=process_list[i];
		bool found=(std::find(pl.begin(),pl.end(),universe.rank())!=pl.end());
		if (found) {
			print("assigning rank",universe.rank(),"to world group",pl);

			SafeMPI::Group group = universe.mpi.comm().Get_group().Incl(pl.size(), &pl[0]);
			SafeMPI::Intracomm comm_group = universe.mpi.comm().Create(group);

			all_worlds.reset(new World(comm_group));
		}
	}
	universe.gop.fence();
	return all_worlds;
}

/// base class
class macro_task_base {
public:
	virtual void run(World& local) = 0;

	virtual void copy_data_in(World& universe, World& local) = 0;

	virtual void copy_data_back(World& universe, World& local) = 0;
	virtual ~macro_task_base() {};

	bool no_task=true;

};


template<typename T>
T copy_to_redistributed(World& local_world, const T& source) {
	print("copying", typeid(source).name());
	return source;
}

template<typename keyT, typename valueT>
WorldContainer<keyT, valueT> copy_to_redistributed(World& local_world,
		const WorldContainer<keyT, valueT>& source) {

	WorldContainer<keyT, valueT> local(local_world);
    for (auto it : source) {
        const auto& key = it.first;
        const auto& value = it.second;
        local.replace(key,value);
    }
    source.get_world().gop.fence();

	return local;
}

template<class F, class...Ts, std::size_t...Is>
void for_each_in_tuple(const std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void for_each_in_tuple(const std::tuple<Ts...> & tuple, F func){
    for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

/// use tuple for the data
template<typename T>
void localize_to_world(World& universe, World& local, const T& data) {

	// to be replaced by MPIArchive
	for_each_in_tuple(data, [&local, &universe](const auto &x) {
//		copy_to_redistributed(local,x);
		std::string filename="dummy"+std::to_string(universe.rank());
        archive::ParallelOutputArchive ar(universe, filename.c_str() , 1);
        ar & x;
        archive::ParallelInputArchive ar2(universe, filename.c_str(), 1);
        ar2 & x;

	});
}



template<typename T>
class macro_task : public macro_task_base {
public:
	double naptime=0.2; // in seconds
	T data;


	macro_task() {}
	macro_task(bool real_task) {
		this->no_task=false;
	}


	void run(World& local) {
		if (local.rank()==0) print("task sleeping, performed by world",local.id());
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<double> dist(0.1, 1.0);
		double nap=dist(mt);

		print("sleeping for",nap);
		myusleep(nap*1.e6);
	}

	void copy_data_in(World& universe, World& local) {
//		localize_to_world(universe,local,data);
	};

	void copy_data_back(World& universe, World& local) {

	};
	~macro_task() {}


    template <typename Archive>
    void serialize(const Archive& ar) {
        ar & data & this->no_task;
    }

};


class WorldSomeThing : public WorldObject<WorldSomeThing> {

};


class MasterPmap : public WorldDCPmapInterface<long> {
public:
    MasterPmap() {}
    ProcessID owner(const long& key) const {return 0;}
};



template<typename taskT>
class macro_taskq : public WorldObject< macro_taskq<taskT> > {
    typedef macro_taskq<taskT> thistype; ///< Type of this class (implementation)
    typedef WorldContainer<long,taskT> dcT; ///< Type of this class (implementation)

    World& universe;
    std::shared_ptr<World> regional_ptr;
	WorldContainer<long,taskT> taskq;

public:
    /// create an empty taskq and initialize the regional world groups
	macro_taskq(World& universe, int nworld)
		  : universe(universe), WorldObject<thistype>(universe),
			taskq(universe,std::shared_ptr< WorldDCPmapInterface<long> > (new MasterPmap())) {

		regional_ptr=(create_worlds(universe,nworld));
	    World& regional=*(regional_ptr.get());
		this->process_pending();
		taskq.process_pending();
	}

	void run_all() {

		World& regional=*(regional_ptr.get());

		universe.gop.fence();
		bool working=true;
		while (working) {
			std::shared_ptr<taskT> task=get_task_from_tasklist(regional);
			task.get()->copy_data_in(universe,regional);
			localize_to_world(universe,regional,task->data);

			if (task.get()) {
				run_task(regional, *task);
			} else {
				working=false;
			}
		}
		universe.gop.fence();
	}

	void add_task(const taskT& task) {
		static int i=0;
		i++;
		int key=(i+universe.rank()*1000);	// unique?
		taskq.replace(key,task);
	};

	std::shared_ptr<taskT> get_task_from_tasklist(World& regional) {

		// only root may pop from the task list
		taskT task;
		if (regional.rank()==0) task=pop();
		regional.gop.broadcast_serializable(task, 0);

        if (task.no_task) return std::shared_ptr<taskT>();
		return std::shared_ptr<taskT>(new taskT(task));	// wait for task to arrive
	}

	void run_task(World& regional, taskT& task) {
		task.run(regional);
	};

	bool master() const {return universe.rank()==0;}

	std::size_t size() const {
		return taskq.size();
	}

	Future<taskT> pop() {
        return this->task(ProcessID(0), &macro_taskq<taskT>::pop_local, this->get_world().rank());
	}

	taskT pop_local(const int requested) {
//		print("pop_local on rank",this->get_world().rank(), "requested from",requested);

		taskT result;
		bool found=false;
		while (not found) {
			auto iter=taskq.begin();
			if (iter==taskq.end()) {
				print("taskq empty");
				break;	// taskq empty
			}

			long key=iter->first;

			typename dcT::accessor acc;
			if (taskq.find(acc,key)) {
				result=acc->second;
				taskq.erase(key);
				break;
			} else {
				print("could not find key",key, "continue searching");
			}
		}
//		print("pop_local: returning",this->get_world().rank(), "requested from",requested);

		return result;
	}

};


int main(int argc, char** argv) {
//    madness::World& universe = madness::initialize(argc,argv);
    initialize(argc, argv);
    World universe(SafeMPI::COMM_WORLD);

    std::cout << "Hello from " << universe.rank() << std::endl;
    universe.gop.fence();
    int nworld=std::min(int(universe.size()),int(3));
    if (universe.rank()==0) print("creating nworld",nworld);

    WorldContainer<int,double> container(universe);
    for (int i=0; i<100; ++i) container.replace(i,double(i));
    print("container local size",container.size());


    typedef macro_task<std::tuple<double,int,FunctionImpl<double,3>* > > taskT;
    macro_taskq<taskT> taskq(universe,nworld);
    if (universe.rank()==0)
    	for (int i=0; i<20; ++i) taskq.add_task(taskT(true));
    universe.gop.fence();

    taskq.run_all();

    madness::finalize();
    return 0;
}


