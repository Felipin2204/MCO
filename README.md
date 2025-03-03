# 📦 MCO

## 📖 Description
INET extension with a Multi-Channel Operation (MCO) framework implementation.

## 📁 Project Structure

```plaintext
📦 vehicular
├── 📂 classifier
│   ├── ApplicationIdToQueueClassifier
│   ├── IWrrClassifier
│   ├── PredefinedCircularPriorityClassifier
│   ├── PredefinedPriorityClassifier
│   ├── RandomSequenceWrrClassifier
│   ├── RandomStartWrrClassifier
│   ├── RandomStartWrrClassifierCC
│   └── WrrClassifierCC
├── 📂 evaluation
│   ├── MCOLoadBalancing
│   ├── MCOLoadBalancingCC
│   ├── MCOLoadBalancingCCMgmt
│   ├── MCOSequentialFilling
│   ├── PredefinedSequentialMCO
│   └── PredefinedSequentialMgmtMCO
├── 📂 flatbuffers
├── 📂 generator
│   ├── DCCTrafficGenerator
│   ├── ExponentialTrafficGenerator
│   ├── NormalizedTrafficGenerator
│   ├── PeriodicTrafficGenerator
│   ├── TrafficGenerator
│   └── UniformSlottedTrafficGenerator
├── 📂 optimal
├── 📂 tr
├── AdjacentChannelInterferenceMonitor
├── ECC68RuralPathLoss
├── FileNodeManager
├── HighwayMobility
├── Ieee80211ScalarRadioInterference
├── IInterferenMonitor
├── IMCO
├── IMgmtMCO
├── INodeManager
├── IVehicleTable
├── MCO
├── MCOPacket
├── MCORateControl
├── MgmtMCO
├── ScalarAnalogModelAdjacentInterference
├── TrafficGeneratorHost
├── TrafficPacket
├── VehicleInfo
├── VehiclesNeighborCache
└── VehicleTable
```

### 📂 `classifier/`
This folder contains classifier implementations for handling different prioritization and packet queueing strategies.

- **`ApplicationIdToQueueClassifier`**: Classifier function that that assigns packets to queues according to application ID.
- **`IWrrClassifier`**: C++ interface for the WRR classifiers of the congestion control case.
- **`PredefinedCircularPriorityClassifier`**: Slightly changed version of PredefinedPriorityClassifier. Once the classifier changes the consumer, the latter is not queued until the classifier completes a full circular round.
- **`PredefinedPriorityClassifier`**: Pushes packets to consumers in a predefined order (sequence), changing to next in sequence only when the corresponding state of the consumer is congested. But when the state of a higher priority consumer is no longer congested, the classifier will classify packets to this consumer.
- **`RandomSequenceWrrClassifier`**: Pushes packets to consumers in a weighted round robin mode. The sequence of the consumers is random for every instance.
- **`RandomStartWrrClassifier`**: Pushes packets to consumers in a weighted round robin mode. The initial consumer is chosen randomly for every instance and then uses the predefined sequence.
- **`RandomStartWrrClassifierCC`**: Slightly changed version of RandomStartWrrClassifier. The only difference is that this classifier avoids congested consumers.
- **`WrrClassifierCC`**: Pushes packets to consumers in a weighted round robin mode avoiding the congested ones.

### 📂 `evaluation/`
This folder contains the channel usage mechanism implementations used for the ETSI TR 103 439 evaluation.

- **`MCOLoadBalancing`**: Load balancing channel usage mechanism implementation.
- **`MCOLoadBalancingCC`**: MCOLoadBalancing implementation with basic congestion control (CBT)
- **`MCOLoadBalancingCCMgmt`**: Management module for the MCOLoadBalancingCC implementation.
- **`MCOSequentialFilling`**: Outdated version of the sequential filling mechanism (with this version you can't define an order).
- **`PredefinedSequentialMCO`**: Sequential filling channel usage mechanism implementation.
- **`PredefinedSequentialMgmtMCO`**: Management module for the PredefinedSequentialMCO implementation.

### 📂 `flatbuffers/`
Flatbuffers folder.

### 📂 `generator/`
This folder contains different traffic generators used for simulations. Each file implements a specific traffic pattern:

- **`DCCTrafficGenerator`**: Generates traffic based on Decentralized Congestion Control (DCC) as defined in ETSI EN 302 663.
- **`ExponentialTrafficGenerator`**: Creates traffic with inter-packet intervals following an exponential distribution.
- **`NormalizedTrafficGenerator`**: With this implementation we let the MCO set the maximum normalized load that the generator can send to the channel. In addition, we select message sizes from a set of fixed sizes.
- **`PeriodicTrafficGenerator`**: Generates traffic at fixed intervals, simulating periodic data transmission.
- **`TrafficGenerator`**: Base class defining common methods and properties for all traffic generators.
- **`UniformSlottedTrafficGenerator`**: Sends packets in uniform time slots, useful for time-slotted transmission systems.

### 📂 `optimal/`
MINOS folder.

### 📂 `tr/`
Everything in this folder was implemented in order to replicate the ETSI TR 103 439 simulations. Modules are a particularization for the one channel scenario, where the nodes only have one NIC. These modules are outdated. Now we can set the number of NICs to one more easily with other forms.

### 📄 Main Files
- **`AdjacentChannelInterferenceMonitor`**: Monitors interference between adjacent channels.
- **`ECC68RuralPathLoss`**: Implements the path loss model for rural environments according to ECC68 specifications.
- **`FileNodeManager`**: Manages network nodes through file-based configuration for simulation scenarios.
- **`HighwayMobility`**: ETSI TR 103 439 mobility model for highway scenarios in vehicular networks.
- **`Ieee80211ScalarRadioInterference`**: 
- **`IInterferenMonitor`**: Interface for interference monitoring implementations.
- **`IMCO`**: Interface for the implementation of modules for MCO mechanism usage.
- **`IMgmtMCO`**: Interface for MCO management modules.
- **`INodeManager`**: Interface for node management modules.
- **`IVehicleTable`**: Interface for vehicle tables modules.
- **`MCO`**: Main module of the Multi-Channel Operation framework.
- **`MCOPacket`**: Specialized packet structure for MCO operations with additional fields.
- **`MCORateControl`**: Manages transmission rate for a certain NIC (It is not functional).
- **`MgmtMCO`**: Management module for the MCO.
- **`ScalarAnalogModelAdjacentInterference`**: Scalar analog model that takes into account adjacent channel interference in the reception (SNIR).
- **`TrafficGeneratorHost`**: Host/Node module used for our simulations.
- **`TrafficPacket`**: Base packet definition for traffic generator modules.
- **`VehicleInfo`**: Data structure for storing comprehensive vehicle neighbor information.
- **`VehiclesNeighborCache`**: Cache for storing and quickly accessing neighboring vehicle information.
- **`VehicleTable`**: Table implementation for storing neigbouring vehicle information.
