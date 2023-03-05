# P2P Chat
## Problem Definition
Develop a peer-to-peer chat application using C++/Qt that satisfies the following requirements:

1. Read a list of peers with associated IP and port from a file in any human-readable format, such as JSON, CSV, or XML.
   * It should be designed in such a way that it can support additional options like a database in the future.
2. Display the list of peers to the user for selection and initiate a chat with the selected peer.
3. To implement network communication you can choose any appropriate options but following criteria should be considered:
   * It must be run in a thread other than the main thread.
   * Supporting text messaging with the ability to extend to other message types like image and file in the future.
4. Implement logging in the application
   * Has an option to write in console or file
   * Includes a timestamp, log type, and message
   * INFO, WARN and ERROR are sufficient for log types

While developing the application, please consider the following guidelines:
1. You can choose to implement the application as either a GUI or Console application. However, the core application should be developed as a
   shared library. This means that if you choose to develop a Console application, another developer should be able to add GUI features in the
   future without modifying the core application (shared lib). Similarly, if you choose to develop a GUI application, other developers should be able to
   implement a different GUI interface without changing the core application. This allows for greater flexibility and maintainability of the codebase.
2. Emphasize clean code with a focus on readability, extensibility, and simplicity.
3. Use Object-Oriented Programming principles.
4. Consider using design patterns, but balance their benefits against the complexity and readability of the code.

Optionally, you may choose to complete the following tasks:
1. Write unit tests for your application, if you are familiar with any unit test frameworks.
2. Use Git to manage your project and deliver your entire application using GitHub. This includes all source code, documentation, and any
   necessary instructions for building and running the application.

## My Solution and Code Structure
### Common Interfaces and Data Structures
We need to create a foundation that makes the interaction of different libraries easy and seamless. At a minimum, this module needs to define the following parts:
* A structure for peers' information, including their name, IP, and port.
* An interface for network operations that different user interfaces (e.g. console, graphical) can use to communicate over the network.
* A callback interface for updating the user interface based on the received network event (e.g. peer connected, message received, peer disconnected).
* Supported message types, which abstract message serialization/deserialization in the network layer and enable interaction between different libraries.

To achieve this, I defined the `inetui` module, which consists only of header files and produces no lib/exe files.

### Reading list of peers
To be able to read a list of peers from a variety of sources and extend the ability to add new ways of data storage and retrieval, 
I added an interface for reading the list of peers and returning a vector of them. This allows us to implement the interface 
according to the data model for each storage method, such as JSON, CSV, and databases.

```
class PeersDataReader {
public:
    virtual std::unique_ptr<std::vector<Peer>> readData() = 0;
    virtual ~PeersDataReader() = default;
};
```

Users can select their desired storage method from the supported methods by using a configuration file. 
For example, I used `/opt/P2pChat/data_sources.json` as the location of the configuration file, and the structure of the file is as follows:

```
{
    "data_sources": {
        "csv": "/opt/P2pChat/peers.csv",
        "sqlite": "/opt/P2pChat/peers.db",
        "json": "/opt/P2pChat/peers.json"
    }
}
```
I also wrote a factory class that reads the configuration file and returns a list of data reader objects. The factory is as follows:

```
class DataReaderFactory {
private:
    std::string configFilePath;
    std::unique_ptr<PeersDataReader> createDataReader(const std::string& type, const std::string& path);
public:
    explicit DataReaderFactory(const std::string &configFilePath);

    std::vector<std::unique_ptr<PeersDataReader>> createDataReadersFromConfig();
};
```
This code is implemented in the dataio module, and after compilation, a shared library is generated that other modules can link to.


### Networking Layer 













### Project Structure


```
+------+      +-------+      +--------+
|  UI  | ---> | inetui| <--- | netlib |<----+
+------+      +-------+      +--------+     |
|                                 |         |
|              +------------------+         |
|              |                            |
|         +--------+      +---------+       |
|         |logging | <--- | dataio  |       |
|         +--------+      +---------+       |
|              |               |            |  
+--------------+---------------+------------+
```


## How to build

## Future Work
