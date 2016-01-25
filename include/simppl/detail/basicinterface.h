#ifndef SIMPPL_DETAIL_BASICINTERFACE_H
#define SIMPPL_DETAIL_BASICINTERFACE_H


// forward decl
struct DBusConnection;


namespace simppl
{

namespace ipc
{

namespace detail
{

struct BasicInterface
{
    inline
    BasicInterface()
     : conn_(nullptr)
    {
        // NOOP
    }

    virtual ~BasicInterface() { /*NOOP*/ }  // FIXME move to .cpp file

    DBusConnection* conn_;
};

}   // namespace detail

}   // namespace ipc

}   // namespace simppl


#endif   // SIMPPL_DETAIL_BASICINTERFACE_H