# AthExHelloWorld

`AthExHelloWorld` demonstrates the basic features of the Athena framework, including:

* The distinction between the role of algorithms and tools, and the correct set-up and structure of each
* Setting the properties of an algorithm via the python configuration layer

It can serve both as a training tool to help new developers to become acquainted with Athena, and also as a template which can be copied and adjusted according to the developer's requirements to build new packages.

The job requires no input and produces no output except log messages.

## Running the example

To run the example you should first set up the ATLAS software and then run the configuration file:

```
asetup Athena,master,latest
python -m AthExHelloWorld.HelloWorldConfig
```

This will run the job according to [this configuration file](https://gitlab.cern.ch/atlas/athena/-/blob/master/Control/AthenaExamples/AthExHelloWorld/python/HelloWorldConfig.py) and will print a number of messages, clearly demonstrating how the algorithm, tool and configuration relate to each other.

## Package description

The package contains a single Athena algorithm called `HelloAlg` and a single AlgTool called `HelloTool`. `HelloAlg` demonstrates the following features of Athena:

* Form of the constructor and declaration of properties set from the configuration layer
* Role and use of the `initialize`, `execute` and `finalize` methods
* Use of the messaging service
* Interaction with public and private tools

`HelloTool` does a simple task - printing a message - but it demonstrates how tools should be set up and interacted with, including the use of an `IAlgTool` interface and `ToolHandles`.
