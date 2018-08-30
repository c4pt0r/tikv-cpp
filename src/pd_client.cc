#include <string>
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include "pd_client.hpp"

namespace tikv {

pd_client::pd_client(const std::string& addr) {
  std::string stripped_address;
  std::string substr("://");
  std::string::size_type i = addr.find(substr);
  if(i != std::string::npos) {
    stripped_address = addr.substr(i+substr.length());
  }

  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(stripped_address, grpc::InsecureChannelCredentials());
  stub_ = pdpb::PD::NewStub(channel);
}

uint64_t pd_client::get_cluster_id() {
    pdpb::GetMembersRequest req;
    pdpb::GetMembersResponse resp;
    grpc::ClientContext ctx;
    grpc::Status st = stub_->GetMembers(&ctx, req, &resp);
    if (st.ok()) {
      std::cout<< "cluster id:" << resp.header().cluster_id() << std::endl << std::endl;
    } else {
      std::cerr<< "err:" << st.error_message() << std::endl;
    }
    // TODO
    return 0;
}

}
