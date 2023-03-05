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

## Source Code Structure
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
