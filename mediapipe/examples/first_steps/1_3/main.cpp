/// Example 1.3 : Joining strings, synchronization, source 
/// By Oleksiy Grechnyev, IT-JIM
/// Here I create a calculator to join two strings
/// Also I introduce a source node (a calculator with no inputs)
/// Note: from now on I define calculators in separate cpp files
/// I also load the graph from a text file graph1_3.pbtxt in this example
/// I use the relative path to graph1_3.pbtxt
/// Run this from mediapipe root directory, like this
/// bazel build --define MEDIAPIPE_DISABLE_GPU=1   //mediapipe/examples/first_steps/1_3
/// bazel-bin/mediapipe/examples/first_steps/1_3/1_3

#include <iostream>
#include <string>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/status.h"


//==============================================================================

mediapipe::Status run(){
    using namespace std;
    // Load the graph config from file 
    string pathGraph("mediapipe/examples/first_steps/1_3/graph1_3.pbtxt");
    string protoG;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(pathGraph, &protoG));
    // Parse it
    mediapipe::CalculatorGraphConfig config;
    if (!mediapipe::ParseTextProto<mediapipe::CalculatorGraphConfig>(protoG, &config)) {
        return absl::InternalError("Cannot parse the graph config !");
    } 

    // Create MP Graph and intialize it with config
    mediapipe::CalculatorGraph graph;
    MP_RETURN_IF_ERROR(graph.Initialize(config));

    // Add observer to "out:
    auto cb = [](const mediapipe::Packet &packet)->mediapipe::Status{
        cout << packet.Timestamp() << ": RECEIVED PACKET " << packet.Get<string>() << endl;
        return mediapipe::OkStatus();
    };
    MP_RETURN_IF_ERROR(graph.ObserveOutputStream("out", cb));

    /// Run the graph
    MP_RETURN_IF_ERROR(graph.StartRun({}));

    // Send input packets to the graph, stream "in", then close it
    // Here I introduce a "time shift" tTshift for "in" timestamps, always 15 packets
    // But timestamps of stream "gen" (generated by StringSourceCalculator)
    // Always start with 0, and there are 17 packets
    // How does MP sync two streams? Try it and find out!
    int tShift = -2;
    for (int i=0; i<15; ++i) {
        mediapipe::Timestamp ts(i + tShift);
        mediapipe::Packet packet = mediapipe::MakePacket<string>("BRIANNA" + to_string(i)).At(ts);
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
    }
    graph.CloseInputStream("in");

    // Wait for the graph to finish
    MP_RETURN_IF_ERROR(graph.WaitUntilDone());
    return mediapipe::OkStatus();
}

//==============================================================================
int main(){
    using namespace std;
    cout << "Example 1.3 : Joining strings, synchronization, source" << endl;
    // Call run(), which return a status
    mediapipe::Status status = run();
    cout << "status = " << status << endl;
    cout << "status.ok() = " << status.ok() << endl;
    return 0;
}