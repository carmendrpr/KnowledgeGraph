#ifndef NewNode_hpp
#define NewNode_hpp

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "knowledge_graph/knowledge_graph.hpp"
#include "my_graph/srv/create_node.hpp"
#include "my_graph/srv/create_edge.hpp"
#include "my_graph/srv/remove_node.hpp"
#include "my_graph/srv/remove_edge.hpp"


using namespace std::chrono_literals;
using std::placeholders::_1;
using std::placeholders::_2;

struct MyNode{
std::string node_name;
std::string node_class;

};

struct MyEdge{
  std::string edge_class;
  std::string source_node;
  std::string target_node;

};


class NewNode : public rclcpp::Node {
public:
  NewNode() : rclcpp::Node("my_knowledgeg_graph"){
    service_create_node=this->create_service<my_graph::srv::CreateNode>("create_node", std::bind(&NewNode::addInfoNode, this, _1,_2));
    service_create_edge=this->create_service<my_graph::srv::CreateEdge>("create_edge", std::bind(&NewNode::addInfoEdge, this, _1,_2));

    //En realidad no estoy segura si tengo que crear dos nuevos .srv para eliminarlos o puedo reutilizar los ya existentes
    // service_remove_node=this->remove_service<my_graph::srv::RemoveNode>("remove_node", std::bind(&NewNode::addInfoNode, this, _1,_2));
    // service_remove_edge=this->remove_service<my_graph::srv::RemoveEdge>("remove_edge", std::bind(&NewNode::addInfoEdge, this, _1,_2));
    
    timer_ = this->create_wall_timer(20ms, std::bind(&NewNode::timerCallback, this));
  }

private:
    std::shared_ptr<knowledge_graph::KnowledgeGraph> graph_;
    knowledge_graph_msgs::msg::Node node_;
    knowledge_graph_msgs::msg::Edge edge_;
    rclcpp::TimerBase::SharedPtr timer_; 
    rclcpp::Service<my_graph::srv::CreateNode>::SharedPtr service_create_node;
    rclcpp::Service<my_graph::srv::CreateEdge>::SharedPtr service_create_edge;
    rclcpp::Service<my_graph::srv::RemoveNode>::SharedPtr service_remove_node;
    rclcpp::Service<my_graph::srv::RemoveEdge>::SharedPtr service_remove_edge;
    MyNode my_node_;
    MyEdge my_edge_;
    bool request_name_received;   
    bool request_edge_received; 
    size_t count_;  
  
    void addInfoNode(const std::shared_ptr<my_graph::srv::CreateNode::Request> request ,const std::shared_ptr<my_graph::srv::CreateNode::Response> response );
    void addInfoEdge(const std::shared_ptr<my_graph::srv::CreateEdge::Request> request,const std::shared_ptr<my_graph::srv::CreateEdge::Response> response );
    void createNode();
    void createEdge();
    void getEdge();
    void getOutEdge();
    void getInEdge();
    void timerCallback();
};

void NewNode::timerCallback(){
  if(request_name_received){
    createNode();
    request_name_received=false;
  };
  if(request_edge_received){
    createEdge();
    getEdge();
    getOutEdge();
    getInEdge();
    request_edge_received=false;
  };

};

void NewNode::addInfoNode(const std::shared_ptr<my_graph::srv::CreateNode::Request> request,const std::shared_ptr<my_graph::srv::CreateNode::Response> response ){
    my_node_.node_name = request->node_name;
    my_node_.node_class = request->node_class;
    request_name_received=true;
    response->resultado=request_name_received;
    RCLCPP_INFO(this->get_logger(),"successfullly received node");
};

void NewNode::createNode(){
  using namespace knowledge_graph;
    this->graph_ = KnowledgeGraph::get_instance(shared_from_this());
    RCLCPP_INFO(this->get_logger(),"successfully built node");
    node_.node_name = std::string(my_node_.node_name);
    node_.node_class = std::string(my_node_.node_class);
    if(this->graph_->update_node(node_,1)==true){
    RCLCPP_INFO(this->get_logger()," successfully update"); 
    };
  RCLCPP_INFO(this->get_logger()," Node Name: %s, Node Class: %s",node_.node_name.c_str(), node_.node_class.c_str());
};

void NewNode::addInfoEdge(const std::shared_ptr<my_graph::srv::CreateEdge::Request> request,const std::shared_ptr<my_graph::srv::CreateEdge::Response> response){
  my_edge_.edge_class = request->edge_class;
  my_edge_.source_node = request->source_node;
  my_edge_.target_node = request->target_node;
  request_edge_received=true;
  response->resultado=request_edge_received;
  RCLCPP_INFO(this->get_logger(),"successfullly received edge");
};    

void NewNode::createEdge(){
  edge_.edge_class = std::string(my_edge_.edge_class);
  edge_.source_node = std::string(my_edge_.source_node);
  edge_.target_node= std::string(my_edge_.target_node);

  if(this->graph_->update_edge(edge_,1)==true){
    RCLCPP_INFO(this->get_logger()," Edge Class: %s, Source Node: %s, Target Node: %s ", edge_.edge_class.c_str(), edge_.source_node.c_str(), edge_.target_node.c_str());
  }
    getEdge();
    getOutEdge();
    getInEdge();
};

void NewNode::getEdge(){
this->graph_->get_edges(edge_.source_node, edge_.target_node);
this->graph_->get_edges(edge_.edge_class);
};

void NewNode::getOutEdge(){
this->graph_->get_out_edges(edge_.source_node);
};

void NewNode::getInEdge(){
this->graph_->get_out_edges(edge_.target_node);
};

#endif