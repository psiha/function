////////////////////////////////////////////////////////////////////////////////
///
/// Boost.Function library
/// 
/// \file function_template.hpp
/// ---------------------------
///
///  Copyright (c) Douglas Gregor  2001-2006
///  Copyright (c) Emil Dotchevski 2007
///  Copyright (c) Domagoj Saric   2010
///
///  Use, modification and distribution is subject to the Boost Software
///  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////

// Note: this header is a header template and must NOT have multiple-inclusion
// protection.
#include <boost/function/detail/prologue.hpp>

#if defined(BOOST_MSVC)
#   pragma warning( push )
#   pragma warning( disable : 4100 ) // "unreferenced formal parameter" (for
                                     // base_empty_handler::operator() that
                                     // ignores all of its parameters
#   pragma warning( disable : 4127 ) // "conditional expression is constant"
#   pragma warning( disable : 4512 ) // "assignment operator could not be generated" (exact_signature_mem_invoker)
#   pragma warning( disable : 4702 ) // "unreachable code" (when calling return
                                     // base_empty_handler::operator() when that
                                     // operator does not return (throws)
#endif       

#define BOOST_FUNCTION_TEMPLATE_PARMS BOOST_PP_ENUM_PARAMS(BOOST_FUNCTION_NUM_ARGS, typename T)

#define BOOST_FUNCTION_TEMPLATE_ARGS BOOST_PP_ENUM_PARAMS(BOOST_FUNCTION_NUM_ARGS, T)

#define BOOST_FUNCTION_PARM(J,I,D) BOOST_PP_CAT(T,I) BOOST_PP_CAT(a,I)

#define BOOST_FUNCTION_PARMS BOOST_PP_ENUM(BOOST_FUNCTION_NUM_ARGS,BOOST_FUNCTION_PARM,BOOST_PP_EMPTY)

#define BOOST_FUNCTION_ARGS BOOST_PP_ENUM_PARAMS(BOOST_FUNCTION_NUM_ARGS, a)

#define BOOST_FUNCTION_ARG_TYPE(J,I,D) \
  typedef BOOST_PP_CAT(T,I) BOOST_PP_CAT(BOOST_PP_CAT(arg, BOOST_PP_INC(I)),_type);

#define BOOST_FUNCTION_ARG_TYPES BOOST_PP_REPEAT(BOOST_FUNCTION_NUM_ARGS,BOOST_FUNCTION_ARG_TYPE,BOOST_PP_EMPTY)

// Comma if nonzero number of arguments
#if BOOST_FUNCTION_NUM_ARGS == 0
#  define BOOST_FUNCTION_COMMA
#else
#  define BOOST_FUNCTION_COMMA ,
#endif // BOOST_FUNCTION_NUM_ARGS > 0

// Class names used in this version of the code
#define BOOST_FUNCTION_FUNCTION BOOST_JOIN(function,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)
#define BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER \
  BOOST_JOIN(void_function_obj_invoker,BOOST_FUNCTION_NUM_ARGS)

#ifndef BOOST_NO_VOID_RETURNS
#  define BOOST_FUNCTION_VOID_RETURN_TYPE void
#  define BOOST_FUNCTION_RETURN(X) X
#else
#  define BOOST_FUNCTION_VOID_RETURN_TYPE boost::detail::function::unusable
#  define BOOST_FUNCTION_RETURN(X) X; return BOOST_FUNCTION_VOID_RETURN_TYPE ()
#endif

#ifndef BOOST_NO_SFINAE
    #define BOOST_FUNCTION_NULL_POINTER_ASSIGNMENT( ReturnType )            \
    ReturnType & operator=( detail::function::useless_clear_type const * )  \
    {                                                                       \
        this->clear();                                                      \
        return *this;                                                       \
    }
#else // BOOST_NO_SFINAE
    #define BOOST_FUNCTION_NULL_POINTER_ASSIGNMENT( ReturnType )            \
    ReturnType & operator=( int const zero )                                \
    {                                                                       \
        BOOST_ASSERT( zero == 0 );                                          \
        this->clear();                                                      \
        return *this;                                                       \
    }
#endif // BOOST_NO_SFINAE

namespace boost {
  namespace detail {
    namespace function {

     template
     <
        typename FunctionObj,
        typename FunctionObjManager,
        typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS
     >
     struct BOOST_FUNCTION_FUNCTION_OBJ_INVOKER : public function_buffer_holder
     {
       // Implementation note:
       //   The buffer argument comes last so that the stack layout in the
       // invoker would be as similar as possible to the one expected by the
       // target (with the assumption of a cdecl-like right-to-left argument
       // order).
       //                                         (25.10.2010.) (Domagoj Saric)
       static R free_invoke( BOOST_FUNCTION_PARMS BOOST_FUNCTION_COMMA function_buffer & buffer )
       {
           // We provide the invoker with a manager with a minimum amount of
           // type information (because it already knows the stored function
           // object it works with, it only needs to get its address from a
           // function_buffer object). Because of this we must cast the pointer
           // returned by FunctionObjManager::functor_ptr() because it can be
           // a plain void * in case of the trivial managers. In case of the
           // trivial ptr manager it is even a void * * so a double static_cast
           // (or a reinterpret_cast) is necessary.
           FunctionObj & functionObject
           (
               *static_cast<FunctionObj *>
               (
                   static_cast<void *>
                   (
                       FunctionObjManager::functor_ptr( buffer )
                   )
               )
           );
           // unwrap_ref is needed because boost::reference_wrapper<T>, unlike
           // the one from std::tr1, does not support callable objects.
           return unwrap_ref( functionObject )( BOOST_FUNCTION_ARGS );
       }

       R bound_invoke( BOOST_FUNCTION_PARMS )
       {
           return free_invoke( BOOST_FUNCTION_ARGS BOOST_FUNCTION_COMMA buffer );
       }
     };

      template
      <
        typename FunctionObj,
        typename FunctionObjManager,
        typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS
      >
      struct BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER : public function_buffer_holder
      {
          static BOOST_FUNCTION_VOID_RETURN_TYPE free_invoke( BOOST_FUNCTION_PARMS BOOST_FUNCTION_COMMA function_buffer & buffer )
          {
              // See the above comments for the non-void invoker.
              FunctionObj & functionObject( *static_cast<FunctionObj *>( static_cast<void *>( FunctionObjManager::functor_ptr( buffer ) ) ) );
              BOOST_FUNCTION_RETURN( unwrap_ref( functionObject )( BOOST_FUNCTION_ARGS ) );
          }

          BOOST_FUNCTION_VOID_RETURN_TYPE bound_invoke( BOOST_FUNCTION_PARMS )
          {
              BOOST_FUNCTION_RETURN( free_invoke( BOOST_FUNCTION_ARGS BOOST_FUNCTION_COMMA buffer ) );
          }
      };
    } // end namespace function
  } // end namespace detail

  template
  <
    typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS,
    class PolicyList
    #if ( BOOST_FUNCTION_NUM_ARGS > 10 )
        = default_policies
    #endif // BOOST_FUNCTION_MAX_ARGS > 10
  >
  class BOOST_FUNCTION_FUNCTION
    : public function_base
#if BOOST_FUNCTION_NUM_ARGS == 1
    , public std::unary_function<T0,R>
#elif BOOST_FUNCTION_NUM_ARGS == 2
    , public std::binary_function<T0,T1,R>
#endif
  {
  private: // Actual policies deduction section.
    //mpl::at<AssocSeq,Key,Default> does not yet exist so...:

    typedef throw_on_empty                                      default_empty_handler ;
    typedef mpl::false_                                         default_nothrow_policy;

    typedef typename mpl::at<PolicyList, empty_handler_t>::type user_specified_empty_handler ;
    typedef typename mpl::at<PolicyList, is_no_throw_t  >::type user_specified_nothrow_policy;

  public: // Public typedefs/introspection section.
#ifndef BOOST_NO_VOID_RETURNS
    typedef R         result_type;
#else
    typedef typename boost::detail::function::function_return_type<R>::type
      result_type;
#endif // BOOST_NO_VOID_RETURNS

    BOOST_STATIC_CONSTANT(int, args = BOOST_FUNCTION_NUM_ARGS);

    // add signature for boost::lambda
    template <typename Args> struct sig { typedef result_type type; };

#if   BOOST_FUNCTION_NUM_ARGS == 1
    typedef T0 argument_type;
#elif BOOST_FUNCTION_NUM_ARGS == 2
    typedef T0 first_argument_type;
    typedef T1 second_argument_type;
#endif

    BOOST_STATIC_CONSTANT( int, arity = BOOST_FUNCTION_NUM_ARGS );
    BOOST_FUNCTION_ARG_TYPES

    typedef BOOST_FUNCTION_FUNCTION self_type;

    typedef typename mpl::if_
            <
                is_same<user_specified_empty_handler, mpl::void_>,
                default_empty_handler,
                user_specified_empty_handler
            >::type base_empty_handler;

  // The nothrow policy and runtime throw detection functionality works only in
  // release mode/with optimizations on. It should also work in debug for plain
  // function pointers with compilers that properly implement the 'exception
  // specification shadow type system' (MSVC 9.0 SP1 does not) - this path yet
  // needs to be tested and properly (re)implemented.
  #ifndef _DEBUG
    typedef typename mpl::if_
        <
            is_same<user_specified_nothrow_policy, mpl::void_>,
            default_nothrow_policy,
            user_specified_nothrow_policy
        >::type nothrow_policy;
  #else
    typedef mpl::false_ nothrow_policy;
  #endif

    typedef R signature_type ( BOOST_FUNCTION_TEMPLATE_ARGS );

  private: // Private implementation types.
    //  We need a specific thin wrapper around the base empty handler that will
    // just consume all the parameters. This way the base empty handler can have
    // one plain simple operator(). As part of ant-code-bloat measures,
    // my_empty_handler is used only when really necessary (with the invoker),
    // otherwise the base_empty_handler type is used.
    struct my_empty_handler : public base_empty_handler
    {
        R operator()( BOOST_FUNCTION_PARMS ) const
        {
            return base_empty_handler:: BOOST_NESTED_TEMPLATE handle_empty_invoke<R>();
        }
    };

    typedef detail::function::vtable vtable_type;

  public: // Public function interface.

    BOOST_FUNCTION_FUNCTION() : function_base( empty_handler_vtable(), base_empty_handler() )
    {
        // Implementation note:
        //   The condition is relaxed for Clang and older GCC that simply seem
        // to have a broken is_statless<> implementation. This should be (more
        // than) safe, for now, because the current code works (or should work)
        // even with non-stateless empty handlers.
        //                                    (28.10.2010.) (Domagoj Saric)
        #if BOOST_WORKAROUND(BOOST_MSVC, >= 1500)
            BOOST_FUNCTION_CLANG_AND_OLD_GCC_BROKEN_STATIC_ASSERT( is_stateless<base_empty_handler>::value );
        #else
            BOOST_FUNCTION_CLANG_AND_OLD_GCC_BROKEN_STATIC_ASSERT( is_empty<base_empty_handler>::value );
        #endif // BOOST_MSVC
    }

    template <typename Functor>
    BOOST_FUNCTION_FUNCTION( Functor const & f,                    BOOST_FUNCTION_ENABLE_IF_NOT_INTEGRAL( Functor, int ) = 0 )
        : function_base( no_eh_state_construction_trick( f ) ) {}

    template <typename Functor, typename Allocator>
    BOOST_FUNCTION_FUNCTION( Functor const & f, Allocator const a, BOOST_FUNCTION_ENABLE_IF_NOT_INTEGRAL( Functor, int ) = 0 )
        : function_base( no_eh_state_construction_trick( f, a ) ) {}

    #ifdef BF_TAKES_FUNCTION_REFERENCES
        BOOST_FUNCTION_FUNCTION( signature_type & plain_function_reference )
            : function_base( no_eh_state_construction_trick( plain_function_reference ) ) { BF_ASSUME( &plain_function_reference ); }
        #ifndef BOOST_NO_SFINAE
            BOOST_FUNCTION_FUNCTION( detail::function::useless_clear_type const * )
                : function_base( empty_handler_vtable(), base_empty_handler() ) {}
        #else
            BOOST_FUNCTION_FUNCTION( int const zero )
                : function_base( empty_handler_vtable(), base_empty_handler() ) { BOOST_ASSERT( zero == 0 ); }
        #endif // BOOST_NO_SFINAE
    #else // BF_TAKES_FUNCTION_REFERENCES
        BOOST_FUNCTION_FUNCTION( signature_type * const plain_function_pointer )
            : function_base( no_eh_state_construction_trick( plain_function_pointer ) ) {}
    #endif // BF_TAKES_FUNCTION_REFERENCES

    BOOST_FUNCTION_FUNCTION( BOOST_FUNCTION_FUNCTION const & f )
        : function_base( static_cast<function_base const &>( f ) ) {}

    /// Clear out a target (replace it with an empty handler), if there is one.
    void clear() { function_base::clear<false, base_empty_handler>( empty_handler_vtable() ); }

    template <typename FunctionObj>
    void assign( FunctionObj const & f                    ) { this->do_assign<false, FunctionObj>( f    ); }

    template <typename FunctionObj, typename Allocator>
    void assign( FunctionObj const & f, Allocator const a ) { this->do_assign<false, FunctionObj>( f, a ); }

    template <signature_type * f>
    void assign()
    {
        this->assign( detail::static_reference_maker<signature_type *>:: BOOST_NESTED_TEMPLATE sref<f>() );
    }

    template <class AClass, R (AClass::*mmf)(BOOST_FUNCTION_TEMPLATE_ARGS)>
    void assign( AClass & object )
    {
        class exact_signature_mem_invoker
        {
        public:
            exact_signature_mem_invoker( AClass & object ) : object( object ) {}
            result_type operator()( BOOST_FUNCTION_PARMS ) const { return (object.*mmf)( BOOST_FUNCTION_ARGS ); }
        private:
            AClass & object;
        };
        this->assign( exact_signature_mem_invoker( object ) );

/* Just an experiment to show how can the current boost::mem_fn implementation
   be 'hacked' to work with custom mem_fn pointer holders (in this case a static
   reference) and/to implement the above.
        typedef static_reference_wrapper
                <
                    R (AClass::*)(BOOST_FUNCTION_TEMPLATE_ARGS),
                    mmf
                > static_mem_fn_reference;

        typedef BOOST_JOIN( detail::function::mem_fn_wrapper::mf, BOOST_FUNCTION_NUM_ARGS )
                <
                    R,
                    AClass
                    BOOST_FUNCTION_COMMA
                    BOOST_FUNCTION_TEMPLATE_ARGS,
                    static_mem_fn_reference
                > mem_fn_wrapper;

        this->assign( bind( mem_fn_wrapper( static_mem_fn_reference() ), &object, _1 ) );
*/
    }

    result_type operator()(BOOST_FUNCTION_PARMS) const
    {
        return invoke( BOOST_FUNCTION_ARGS BOOST_FUNCTION_COMMA nothrow_policy() );
    }


    // ...this one is perhaps no longer needed (the one below can probably "take
    // over"...
    BOOST_FUNCTION_FUNCTION & operator=( BOOST_FUNCTION_FUNCTION const & f )
    {
        this->assign( f );
        return *this;
    }

    template <typename Functor>
    BOOST_FUNCTION_ENABLE_IF_NOT_INTEGRAL( Functor, BOOST_FUNCTION_FUNCTION & )
    operator=( Functor const & f )
    {
      this->assign( f );
      return *this;
    }

    #ifdef BF_TAKES_FUNCTION_REFERENCES
        BOOST_FUNCTION_FUNCTION & operator=( signature_type & plain_function_reference )
        {
            BF_ASSUME( &plain_function_reference );
            this->assign( plain_function_reference );
            return *this;
        }

        BOOST_FUNCTION_NULL_POINTER_ASSIGNMENT( BOOST_FUNCTION_FUNCTION )
    #else // BF_TAKES_FUNCTION_REFERENCES
        BOOST_FUNCTION_FUNCTION & operator=( signature_type * const plain_function_pointer )
        {
            this->assign( plain_function_pointer );
            return *this;
        }
    #endif // BF_TAKES_FUNCTION_REFERENCES

    void swap( BOOST_FUNCTION_FUNCTION & other )
    {
        BOOST_STATIC_ASSERT( sizeof( BOOST_FUNCTION_FUNCTION ) == sizeof( function_base ) );
        return function_base::swap<base_empty_handler>( other, empty_handler_vtable() );
    }

  public:
    BOOST_SAFE_BOOL_FOR_TEMPLATE_FROM_FUNCTION( BOOST_FUNCTION_FUNCTION, !empty );
    bool operator!() const { return this->empty(); }

private:
    static vtable_type const & empty_handler_vtable() { return vtable_for_functor<detail::function::fallocator<base_empty_handler>, base_empty_handler>( my_empty_handler() ); }

    template <class F>
    static bool nothrow_test( F & f BOOST_FUNCTION_COMMA BOOST_FUNCTION_PARMS )
    {
        f( BOOST_FUNCTION_ARGS );
        return true;
    }

    template <class F>
    static bool throw_test( F & f BOOST_FUNCTION_COMMA BOOST_FUNCTION_PARMS )
    {
        try
        {
            f( BOOST_FUNCTION_ARGS );
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // Implementation note:
    //   This function relies on "link-time optimizer procedure analysis
    // introspection" to detect whether the compiler actually recognized a
    // function as nothrow (expecting that in this case it will generate the
    // same code for both functions which the linker will then merge giving them
    // effectively identical addresses).
    //   The method was tested to work in 32 bit release mode with MSVC++ 8.0,
    // 9.0 and 10.0, it does not seem to work in 64 bit mode with the same
    // compiler (the compiler generates the catch(...) clause code even for the
    // most trivial and obviously nothrow functions).
    //   Because this check relies on such undocumented non-standard trickery,
    // for unsupported targets the function simply returns true and it is up to
    // the user to make sure that only really nothrow targets are assigned to
    // nothrow marked boost::functions.
    //                                        (01.11.2010.) (Domagoj Saric)
    template <class F>
    static bool is_nothrow()
    {
        #if !defined( _DEBUG ) && defined( BOOST_MSVC ) && defined( _M_IX86 )
            typedef bool ( throw_test_signature ) ( typename unwrap_reference<F>::type & BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_ARGS );
            return detail::function::is_nothrow_helper
                   <
                       throw_test_signature,
		               &BOOST_FUNCTION_FUNCTION:: BOOST_NESTED_TEMPLATE nothrow_test<typename unwrap_reference<F>::type>,
		               &BOOST_FUNCTION_FUNCTION:: BOOST_NESTED_TEMPLATE throw_test  <typename unwrap_reference<F>::type>
		           >::is_nothrow;

		    #if BOOST_WORKAROUND( BOOST_MSVC, == 1400 )
		        // MSVC++ 8.0 (SP1) linker reports "unresolved external symbol"
		        // errors (which are erroneous themselves as the symbols do exist
		        // and the binary works properly when linked with /FORCE) for the
		        // test functions so we force their inclusion here. This is below
		        // the return statement so as not to generate any code, this does
		        // not generate any warning with the targeted compiler.
			    static throw_test_signature * const nothrower( &nothrow_test<unwrap_reference<F>::type> );
			    static throw_test_signature * const   thrower( &throw_test  <unwrap_reference<F>::type> );
		    #endif
        #else
            return true;
        #endif // supported no-throw-check platforms
    }

    #ifdef BF_HAS_NOTHROW
        BF_NOTHROW
    #endif
    result_type invoke( BOOST_FUNCTION_PARMS BOOST_FUNCTION_COMMA mpl::true_ /*no throw invoker*/ ) const
    #ifndef BF_HAS_NOTHROW
        throw()
    #endif
    {
        return do_invoke( BOOST_FUNCTION_ARGS BOOST_FUNCTION_COMMA detail::function::thiscall_optimization_available() );
    }

    BF_FORCEINLINE
    result_type invoke( BOOST_FUNCTION_PARMS BOOST_FUNCTION_COMMA mpl::false_ /*throwable invoker*/ ) const
    {
        return do_invoke( BOOST_FUNCTION_ARGS BOOST_FUNCTION_COMMA detail::function::thiscall_optimization_available() );
    }

    BF_FORCEINLINE
    result_type do_invoke( BOOST_FUNCTION_PARMS BOOST_FUNCTION_COMMA mpl::true_ /*this call*/ ) const
    {
        typedef result_type (detail::function::function_buffer::* invoker_type)(BOOST_FUNCTION_TEMPLATE_ARGS);
        return (functor().*(get_vtable(). BOOST_NESTED_TEMPLATE invoker<invoker_type>()))(BOOST_FUNCTION_ARGS);
    }

    BF_FORCEINLINE
    result_type do_invoke( BOOST_FUNCTION_PARMS BOOST_FUNCTION_COMMA mpl::false_ /*free call*/ ) const
    {
        typedef result_type (* invoker_type)( BOOST_FUNCTION_TEMPLATE_ARGS BOOST_FUNCTION_COMMA detail::function::function_buffer & );
        return get_vtable(). BOOST_NESTED_TEMPLATE invoker<invoker_type>()( BOOST_FUNCTION_ARGS BOOST_FUNCTION_COMMA functor() );
    }


    //  This overload should not actually be for a 'complete' BOOST_FUNCTION_FUNCTION as it is enough
	// for the signature template parameter to be the same (and therefor the vtable is the same, with
	// a possible exception being the case of an empty source as empty handler vtables depend on the
	// policy as well as the signature).
    template <typename Allocator, typename ActualFunctor>
    static vtable_type const & vtable_for_functor_aux
    (
        mpl::true_, // is a boost::function<>
        BOOST_FUNCTION_FUNCTION const & functor
    )
    {
      BOOST_STATIC_ASSERT(( is_base_of<BOOST_FUNCTION_FUNCTION, ActualFunctor>::value ));
      return functor.get_vtable();
    }

    template <typename Allocator, typename ActualFunctor, typename StoredFunctor>
    static vtable_type const & vtable_for_functor_aux
    (
        mpl::false_, // is a boost::function<>
        StoredFunctor const & /*functor*/
    )
    {
      using namespace detail::function;

      // A minimally typed manager is used for the invoker (anti-code-bloat).
      typedef typename get_functor_manager
              <
                StoredFunctor,
                Allocator
              >::type invoker_manager_type;

      // For the empty handler we use the manager for the base_empty_handler not
      // my_empty_handler (anti-code-bloat) because they only differ in the
      // operator() member function which is irrelevant for/not used by the
      // manager.
      typedef typename get_typed_functor_manager
              <
                ActualFunctor,
                typename mpl::if_
                <
                  is_same<ActualFunctor, base_empty_handler>,
                  ActualFunctor,
                  StoredFunctor
                >::type,
                Allocator
              >::type manager_type;

      typedef typename mpl::if_
              <
                is_void<R>,
                BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER
                <
                    StoredFunctor,
                    invoker_manager_type,
                    R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_ARGS
                >,
                BOOST_FUNCTION_FUNCTION_OBJ_INVOKER
                <
                    StoredFunctor,
                    invoker_manager_type,
                    R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_ARGS
                >
            >::type invoker_type;

      BOOST_STATIC_ASSERT
      ((
        is_same<ActualFunctor, base_empty_handler>::value
            ==
        is_same<StoredFunctor, my_empty_handler  >::value
      ));
      typedef is_same<ActualFunctor, base_empty_handler> is_empty_handler;
      return vtable_holder<invoker_type, manager_type, is_empty_handler>::stored_vtable;
    }

    template <typename Allocator, typename ActualFunctor, typename StoredFunctor>
    static vtable_type const & vtable_for_functor( StoredFunctor const & functor )
    {
        return vtable_for_functor_aux<Allocator, ActualFunctor>
        (
            is_base_of<BOOST_FUNCTION_FUNCTION, StoredFunctor>(),
            functor
        );        
    }

    // ...direct actually means whether to skip pre-destruction (when not
    // assigning but constructing) so it should probably be renamed to
    // pre_destroy or the whole thing solved in some smarter way...
    template <bool direct, typename FunctionObj, typename Allocator>
    void do_assign( FunctionObj const & f, Allocator const a )
    {
        typedef typename detail::function::get_function_tag<FunctionObj>::type tag;
        dispatch_assign<direct, FunctionObj>( f, a, tag() );
    }

    template <bool direct, typename FunctionObj>
    void do_assign( FunctionObj const & f ) { do_assign<direct, FunctionObj>( f, detail::function::fallocator<FunctionObj>() ); }

    template <bool direct, typename FunctionObj, typename Allocator>
    void dispatch_assign( FunctionObj const & f, Allocator const a, detail::function::function_obj_tag     ) { do_assign<direct,          FunctionObj      >( f      ,        f   , a ); }
    // Explicit support for member function objects, so we invoke through
    // mem_fn() but retain the right target_type() values.
    template <bool direct, typename FunctionObj, typename Allocator>
    void dispatch_assign( FunctionObj const & f, Allocator const a, detail::function::member_ptr_tag       ) { do_assign<direct,          FunctionObj      >( f      , mem_fn( f ), a ); }
    template <bool direct, typename FunctionObj, typename Allocator>
    void dispatch_assign( FunctionObj const & f, Allocator const a, detail::function::function_obj_ref_tag ) { do_assign<direct, typename FunctionObj::type>( f.get(),         f  , a ); }
    template <bool direct, typename FunctionObj, typename Allocator>
    void dispatch_assign( FunctionObj const & f, Allocator const a, detail::function::function_ptr_tag     )
    {
        // Implementation note:
        //   Plain function pointers need special care because when assigned
        // using the syntax without the ampersand they wreck havoc with certain
        // compilers, causing either compilation failures or broken runtime
        // behaviour, e.g. not invoking the assigned target with GCC 4.0.1 or
        // causing access-violation crashes with MSVC (tested 8 and 10).
        //                                    (03.11.2010.) (Domagoj Saric)
        typedef typename add_pointer
        <
            typename remove_const
            <
                typename remove_pointer<FunctionObj>::type
            >::type
        >::type non_const_function_pointer_t;

        // Implementation note:
        //   Single place to handle int-assignment for non SFINAE enabled
        // compilers.
        //                                    (05.11.2010.) (Domagoj Saric)
        #ifdef BOOST_NO_SFINAE
            typedef typename mpl::if_
            <
                is_integral<FunctionObj>,
                signature_type *,
                non_const_function_pointer_t
            >::type correct_type_t;

            if ( is_integral<FunctionObj>::value )
            {
                BOOST_ASSERT( reinterpret_cast<int const &>( f ) == 0 );
                function_base::clear<direct, base_empty_handler>( empty_handler_vtable() );
            }
            else
            {
                // Implementation note:
                //   Ugh...(exactly) this seems to work with the 'ampersandless'
                // syntax.
                //                            (05.11.2010.) (Domagoj Saric)
                do_assign<direct, correct_type_t, correct_type_t>( reinterpret_cast<correct_type_t>( f ), reinterpret_cast<correct_type_t>( f ), a );
            }
        #else
            typedef non_const_function_pointer_t correct_type_t;
            do_assign<direct, non_const_function_pointer_t, non_const_function_pointer_t>( f, f, a );
        #endif
    }

    template <bool direct, typename ActualFunctor, typename StoredFunctor, typename ActualFunctorAllocator>
    void do_assign( ActualFunctor const &, StoredFunctor const & stored_functor, ActualFunctorAllocator const a )
    {
        if
        (
            ( nothrow_policy::value == true                              ) &&
            // Assume other copies of the same type of boost::function did
            // their job in detecting (no)throw function objects.
            // This function-object-type-specific detection should probably
            // be moved into the tag dispatched assigns (where 'exception
            // specification shadow type system' detection for function
            // pointer should be implemented also).
            ( !is_base_of<BOOST_FUNCTION_FUNCTION, ActualFunctor>::value ) &&
            ( !is_nothrow<StoredFunctor>()                               )
        )
        {
            // This implementation inserts two calls to destroy() (the one here
            // for the clear and the one, for the else case, further down in 
            // assign) when the nothrow policy is specified...this should be
            // fixed...
            BOOST_ASSERT( is_nothrow<my_empty_handler>() );
            base_empty_handler /*const*/ emptyHandler;
            function_base::assign<direct, base_empty_handler>
            (
                emptyHandler,
                empty_handler_vtable(),
                empty_handler_vtable(),
                detail::function::fallocator<base_empty_handler>(),
                mpl::false_() /*not assigning another boost::function*/
            );
            emptyHandler. BOOST_NESTED_TEMPLATE handle_empty_invoke<R>();
        }
        else
        {
            typedef typename ActualFunctorAllocator:: BOOST_NESTED_TEMPLATE rebind<StoredFunctor>::other StoredFunctorAllocator;
            function_base::assign<direct, base_empty_handler>
            (
                stored_functor,
                vtable_for_functor<StoredFunctorAllocator, ActualFunctor>( stored_functor ),
                empty_handler_vtable(),
                StoredFunctorAllocator( a ),
                is_base_of<BOOST_FUNCTION_FUNCTION, StoredFunctor>() /*are we assigning another boost::function*/
            );
        }
    }

    // Implementation note:
    //   Simply default-constructing funciton_base and then performing proper
    // initialization in the body of the derived class constructor has
    // unfortunate efficiency implications because it creates unnecessary
    // EH states (=unnecessary bloat) in case of non-trivial
    // (i.e. fallible/throwable) constructors of derived classes (when
    // constructing from/with complex function objects).
    //   In such cases the compiler has to generate EH code to call the
    // (non-trivial) function_base destructor if the derived-class constructor
    // fails after function_base is already constructed. This is completely
    // redundant because initially function_base is/was always initialized with
    // the empty handler for which no destruction is necessary but the compiler
    // does not see this because of the indirect vtable call.
    //   Because of the above issue, the helper functions below are used as a
    // quick-hack to actually construct the function_base/
    // BOOST_FUNCTION_FUNCTION object before the function_base constructor is
    // called. The entire object is also cleared beforehand in debugging builds
    // to allow checking that the vtable and/or the function_buffer are not used
    // before being initialized.
    //                                        (02.11.2010.) (Domagoj Saric)
    /// \todo Devise a cleaner way to deal with all of this (maybe move/add more
    /// template methods to function_base so that it can call assign methods
    /// from its template constructors thereby moving all construction code
    /// there).
    ///                                       (02.11.2010.) (Domagoj Saric)
    template <typename FunctionObj, typename Allocator>
    detail::function::vtable const & no_eh_state_construction_trick( FunctionObj const & f, Allocator const a )
    {
        detail::function::debug_clear( *this );
        do_assign<true>( f, a );
        return function_base::get_vtable();
    }

    template <typename FunctionObj>
    detail::function::vtable const & no_eh_state_construction_trick( FunctionObj const & f )
    {
        detail::function::debug_clear( *this );
        do_assign<true>( f, detail::function::fallocator<FunctionObj>() );
        return function_base::get_vtable();
    }

    #ifdef BF_TAKES_FUNCTION_REFERENCES
        detail::function::vtable const & no_eh_state_construction_trick( signature_type & plain_function_reference )
        {
            BF_ASSUME( &plain_function_reference );
            detail::function::debug_clear( *this );
            do_assign<true>( plain_function_reference );
            return function_base::get_vtable();
        }
    #endif // BF_TAKES_FUNCTION_REFERENCES
  };

  template<typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS, class PolicyList>
  inline void swap(BOOST_FUNCTION_FUNCTION<
                     R BOOST_FUNCTION_COMMA
                     BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList
                   >& f1,
                   BOOST_FUNCTION_FUNCTION<
                     R BOOST_FUNCTION_COMMA
                     BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList
                   >& f2)
  {
    f1.swap(f2);
  }

// Poison comparisons between boost::function objects of the same type.
template<typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS, class PolicyList>
  void operator==(const BOOST_FUNCTION_FUNCTION<
                          R BOOST_FUNCTION_COMMA
                          BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList>&,
                  const BOOST_FUNCTION_FUNCTION<
                          R BOOST_FUNCTION_COMMA
                          BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList>&);
template<typename R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_PARMS, class PolicyList>
  void operator!=(const BOOST_FUNCTION_FUNCTION<
                          R BOOST_FUNCTION_COMMA
                          BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList>&,
                  const BOOST_FUNCTION_FUNCTION<
                          R BOOST_FUNCTION_COMMA
                          BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList>& );

#if !defined(BOOST_FUNCTION_NO_FUNCTION_TYPE_SYNTAX)

#if BOOST_FUNCTION_NUM_ARGS == 0
#define BOOST_FUNCTION_PARTIAL_SPEC R (void), PolicyList
#else
#define BOOST_FUNCTION_PARTIAL_SPEC R (BOOST_PP_ENUM_PARAMS(BOOST_FUNCTION_NUM_ARGS,T)), PolicyList
#endif

template<typename R BOOST_FUNCTION_COMMA
         BOOST_FUNCTION_TEMPLATE_PARMS, class PolicyList>
class function<BOOST_FUNCTION_PARTIAL_SPEC>
  : public BOOST_FUNCTION_FUNCTION<R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList>
{
  typedef BOOST_FUNCTION_FUNCTION<R BOOST_FUNCTION_COMMA BOOST_FUNCTION_TEMPLATE_ARGS, PolicyList> base_type;
  typedef function self_type;

  struct clear_type {};

public:
  function() {}

  template <typename Functor>
  function( Functor const & f,                    BOOST_FUNCTION_ENABLE_IF_NOT_INTEGRAL( Functor, int ) = 0 ) : base_type( f    ) {}

  template <typename Functor, typename Allocator>
  function( Functor const & f, Allocator const a, BOOST_FUNCTION_ENABLE_IF_NOT_INTEGRAL( Functor, int ) = 0 ) : base_type( f, a ) {}

  #ifdef BF_TAKES_FUNCTION_REFERENCES
    function( typename base_type::signature_type &       plain_function_reference ) : base_type( plain_function_reference ) {}
    #ifndef BOOST_NO_SFINAE
        function( detail::function::useless_clear_type const * ) : base_type() {}
    #endif // BOOST_NO_SFINAE
  #else // BF_TAKES_FUNCTION_REFERENCES
    function( typename base_type::signature_type * const plain_function_pointer   ) : base_type( plain_function_pointer   ) {}
  #endif // BF_TAKES_FUNCTION_REFERENCES

  function( self_type const & f ) : base_type( static_cast<base_type const &>( f ) ) {}

  function( base_type const & f ) : base_type( static_cast<base_type const &>( f ) ) {}

  // The distinction between when to use BOOST_FUNCTION_FUNCTION and
  // when to use self_type is obnoxious. MSVC cannot handle self_type as
  // the return type of these assignment operators, but Borland C++ cannot
  // handle BOOST_FUNCTION_FUNCTION as the type of the temporary to
  // construct.
  self_type & operator=( self_type const & f )
  {
      this->assign( f );
      return *this;
  }

  template <typename Functor>
  BOOST_FUNCTION_ENABLE_IF_NOT_INTEGRAL( Functor, self_type & )
  operator=( Functor const & f )
  {
    this-> BOOST_NESTED_TEMPLATE assign<Functor>( f );
    return *this;
  }

  #ifdef BF_TAKES_FUNCTION_REFERENCES
      self_type & operator=( typename base_type::signature_type & plain_function_reference )
      {
          BF_ASSUME( &plain_function_reference );
          this->assign( plain_function_reference );
          return *this;
      }

      BOOST_FUNCTION_NULL_POINTER_ASSIGNMENT( self_type )
  #else // BF_TAKES_FUNCTION_REFERENCES
      self_type & operator=( typename base_type::signature_type * const plain_function_pointer )
      {
          this->assign( plain_function_pointer );
          return *this;
      }
  #endif // BF_TAKES_FUNCTION_REFERENCES
};

#undef BOOST_FUNCTION_PARTIAL_SPEC
#endif // have partial specialization

} // end namespace boost

// Cleanup after ourselves...
#undef BOOST_FUNCTION_COMMA
#undef BOOST_FUNCTION_FUNCTION
#undef BOOST_FUNCTION_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_VOID_FUNCTION_OBJ_INVOKER
#undef BOOST_FUNCTION_TEMPLATE_PARMS
#undef BOOST_FUNCTION_TEMPLATE_ARGS
#undef BOOST_FUNCTION_PARMS
#undef BOOST_FUNCTION_PARM
#undef BOOST_FUNCTION_ARGS
#undef BOOST_FUNCTION_ARG_TYPE
#undef BOOST_FUNCTION_ARG_TYPES
#undef BOOST_FUNCTION_VOID_RETURN_TYPE
#undef BOOST_FUNCTION_RETURN

#undef BOOST_FUNCTION_NULL_POINTER_ASSIGNMENT

#if defined(BOOST_MSVC)
#   pragma warning( pop )
#endif       
