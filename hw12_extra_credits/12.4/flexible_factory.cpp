#include "flexible_factory.h"
#include <iostream>
#include <memory>
#include <string>

using namespace std;
using namespace cspp51045;

// Abstract train car classes
struct Locomotive {
    virtual void display() = 0;
    virtual double getHorsepower() const = 0;
    virtual ~Locomotive() = default;
};

struct FreightCar {
    virtual void display() = 0;
    virtual long getCapacity() const = 0;
    virtual ~FreightCar() = default;
};

struct Caboose {
    virtual void display() = 0;
    virtual ~Caboose() = default;
};

// Concrete model train implementations
struct ModelLocomotive : public Locomotive {
    ModelLocomotive(double horsepower) : horsepower_(horsepower) {
        cout << "Creating model locomotive with " << horsepower << " HP" << endl;
    }
    
    void display() override {
        cout << "Model locomotive with " << horsepower_ << " HP" << endl;
    }
    
    double getHorsepower() const override {
        return horsepower_;
    }
    
private:
    double horsepower_;
};

struct ModelFreightCar : public FreightCar {
    ModelFreightCar(long capacity) : capacity_(capacity) {
        cout << "Creating model freight car with " << capacity << " capacity" << endl;
    }
    
    void display() override {
        cout << "Model freight car with " << capacity_ << " capacity" << endl;
    }
    
    long getCapacity() const override {
        return capacity_;
    }
    
private:
    long capacity_;
};

struct ModelCaboose : public Caboose {
    ModelCaboose() {
        cout << "Creating model caboose" << endl;
    }
    
    void display() override {
        cout << "Model caboose" << endl;
    }
};

// Concrete real train implementations
struct RealLocomotive : public Locomotive {
    RealLocomotive(double horsepower) : horsepower_(horsepower) {
        cout << "Creating real locomotive with " << horsepower << " HP" << endl;
    }
    
    void display() override {
        cout << "Real locomotive with " << horsepower_ << " HP" << endl;
    }
    
    double getHorsepower() const override {
        return horsepower_;
    }
    
private:
    double horsepower_;
};

struct RealFreightCar : public FreightCar {
    RealFreightCar(long capacity) : capacity_(capacity) {
        cout << "Creating real freight car with " << capacity << " capacity" << endl;
    }
    
    void display() override {
        cout << "Real freight car with " << capacity_ << " capacity" << endl;
    }
    
    long getCapacity() const override {
        return capacity_;
    }
    
private:
    long capacity_;
};

struct RealCaboose : public Caboose {
    RealCaboose() {
        cout << "Creating real caboose" << endl;
    }
    
    void display() override {
        cout << "Real caboose" << endl;
    }
};

// Define the abstract factory with signatures
using TrainFactory = flexible_abstract_factory<
    Locomotive(double), 
    FreightCar(long), 
    Caboose()
>;

// Define concrete factory for model trains
using ModelTrainFactory = flexible_concrete_factory<
    TrainFactory,
    concrete_pair<TrainFactory, Locomotive(double), ModelLocomotive>,
    concrete_pair<TrainFactory, FreightCar(long), ModelFreightCar>,
    concrete_pair<TrainFactory, Caboose(), ModelCaboose>
>;

// Define concrete factory for real trains
using RealTrainFactory = flexible_concrete_factory<
    TrainFactory,
    concrete_pair<TrainFactory, Locomotive(double), RealLocomotive>,
    concrete_pair<TrainFactory, FreightCar(long), RealFreightCar>,
    concrete_pair<TrainFactory, Caboose(), RealCaboose>
>;

int main() {
    cout << "Creating model train:" << endl;
    unique_ptr<TrainFactory> modelFactory = make_unique<ModelTrainFactory>();
    
    unique_ptr<Locomotive> modelLoco = modelFactory->create<Locomotive>(75.5);
    unique_ptr<FreightCar> modelFreight = modelFactory->create<FreightCar>(250L);
    unique_ptr<Caboose> modelCaboose = modelFactory->create<Caboose>();
    
    cout << "\nDisplaying model train components:" << endl;
    modelLoco->display();
    modelFreight->display();
    modelCaboose->display();
    
    cout << "\n\nCreating real train:" << endl;
    unique_ptr<TrainFactory> realFactory = make_unique<RealTrainFactory>();
    
    unique_ptr<Locomotive> realLoco = realFactory->create<Locomotive>(12000.0);
    unique_ptr<FreightCar> realFreight = realFactory->create<FreightCar>(10000L);
    unique_ptr<Caboose> realCaboose = realFactory->create<Caboose>();
    
    cout << "\nDisplaying real train components:" << endl;
    realLoco->display();
    realFreight->display();
    realCaboose->display();
    
    return 0;
}