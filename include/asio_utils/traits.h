﻿#pragma once

#include <type_traits>

#include "asio_utils/utils.h"

namespace eld
{
    namespace traits
    {
        template<typename T>
        struct connection
        {
            using config_type = typename T::config_type;
        };
    }

    /**
     * Contains customization points with default implementations.
     */
    namespace custom
    {
        namespace detail
        {
            template<typename TL, typename TR, typename /*has_operator_==*/ = void>
            struct default_false_compare
            {
                constexpr static bool value(const TL &, const TR &) { return false; }
            };

            template<typename TL, typename TR>
            struct default_false_compare<
                TL,
                TR,
                util::void_t<decltype(std::declval<TL>() == std::declval<TR>())>>
            {
                constexpr static bool value(const TL &lhv, const TR &rhv) { return lhv == rhv; }
            };

        }

        // TODO: add make_completion function
        struct connection_asio_impl
        {
            using error_type = asio::error_code;
            using buffer_type = std::array<uint8_t, 2048>;

            struct error
            {
                constexpr static auto operation_aborted()
                {
                    return asio::error::operation_aborted;
                }

                constexpr static auto already_started()
                {
                    return asio::error::already_started;
                }
            };

            template<typename ConnectionT>
            static constexpr typename traits::connection<ConnectionT>::config_type get_config(
                const ConnectionT &connection)
            {
                return connection.getConfig();
            }

            template<typename ConfigTL, typename ConfigTR>
            static constexpr bool compare_configs(const ConfigTL &lhv, const ConfigTR &rhv)
            {
                return detail::default_false_compare<ConfigTL, ConfigTR>::value(lhv, rhv);
            }

            template<typename T>
            static constexpr void cancel(T &t)
            {
                t.cancel();
            }

            template<typename T, typename ConfigT>
            static void configure(T &t, ConfigT &&config)
            {
                t.configure(std::forward<ConfigT>(config));
            }

            // TODO: check completion signature
            template<typename Connection, typename MutableBuffer, typename CompletionT>
            static auto async_receive(Connection &connection,
                               MutableBuffer &&mutableBuffer,
                               CompletionT &&completion)
            {
                return connection.async_receive(
                    asio::buffer(std::forward<MutableBuffer>(mutableBuffer)),
                    std::forward<CompletionT>(completion));
            }

            // TODO: check completion signature
            template<typename Connection, typename ConstBuffer, typename CompletionT>
            static auto async_send(Connection &connection, ConstBuffer &&constBuffer, CompletionT &&completion)
            {
                return asio::async_write(connection,
                                         std::forward<ConstBuffer>(constBuffer),
                                         std::forward<CompletionT>(completion));
            }

            template<typename ConnectionT>
            static constexpr bool remote_host_has_disconnected(const ConnectionT &, const error_type &errorCode)
            {
                return errorCode == asio::error::eof ||   //
                       errorCode == asio::error::connection_reset;
            }

        };
    }
}