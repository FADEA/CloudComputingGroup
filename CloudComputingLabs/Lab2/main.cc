#include "server.h"


int main(){

    /*创建监听套接字 */
    int sockfd=create_sockfd();

    /*接受客户端连接 */
    int fd=accept(sockfd,NULL,NULL);
    cout<<"有客户端连接"<<endl;

    /*处理客户端请求 */
    handle_request(fd);

    /*关闭套接字 */
    close(fd);
    close(sockfd);
}