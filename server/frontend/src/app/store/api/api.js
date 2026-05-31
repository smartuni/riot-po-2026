import { createApi, fetchBaseQuery } from '@reduxjs/toolkit/query/react';
import { setCookie, eraseCookie } from '../../../shared/utils/cookie';

export const api = createApi({
  reducerPath: 'api',
  baseQuery: fetchBaseQuery({
    baseUrl: import.meta.env.VITE_API_BASE_URL ?? '',
    prepareHeaders: (headers, { getState }) => {
      const token = getState()?.auth?.token;
      if (token) {
        headers.set('Authorization', `Bearer ${token}`);
      }
      headers.set('Content-Type', 'application/json');
      return headers;
    },
  }),
  tagTypes: ['Gate', 'Activity', 'Notification', 'Auth'],
  endpoints: (builder) => ({
    // ── Auth ──────────────────────────────────────────────
    login: builder.mutation({
      query: (credentials) => ({
        url: '/api/auth/login',
        method: 'POST',
        body: credentials,
      }),
      onQueryStarted: async (_, { queryFulfilled, dispatch }) => {
        const { data } = await queryFulfilled;
        dispatch({ type: 'auth/setToken', payload: data.token });
        setCookie('jwt', data.token);
      },
    }),
    register: builder.mutation({
      query: (registrationData) => ({
        url: '/api/auth/register',
        method: 'POST',
        body: registrationData,
      }),
      onQueryStarted: async (_, { queryFulfilled, dispatch }) => {
        const { data } = await queryFulfilled;
        dispatch({ type: 'auth/setToken', payload: data.token });
        setCookie('jwt', data.token);
      },
    }),
    getUserDetails: builder.query({
      query: () => '/api/auth/user-details',
      providesTags: ['Auth'],
    }),
    updateUserDetails: builder.mutation({
      query: (body) => ({
        url: '/api/auth/user-change',
        method: 'PUT',
        body,
      }),
      invalidatesTags: ['Auth'],
    }),
    logout: builder.mutation({
      query: () => ({
        url: '/api/auth/logout',
        method: 'POST',
      }),
      onQueryStarted: async (_, { queryFulfilled, dispatch }) => {
        // Clear client auth state regardless of whether the server call
        // succeeds — a failed logout request must still log the user out
        // locally, otherwise the stale token keeps protected pages rendering.
        try {
          await queryFulfilled;
        } finally {
          dispatch({ type: 'auth/clearToken' });
          eraseCookie('jwt');
        }
      },
    }),

    // ── Gates ─────────────────────────────────────────────
    getGates: builder.query({
      query: () => '/api/gates',
      providesTags: (result) => result ? result.map((gate) => ({ type: 'Gate', id: gate.id })) : [{ type: 'Gate' }],
    }),
    createGate: builder.mutation({
      query: (newGateData) => ({
        url: '/api/add-gate-ui',
        method: 'POST',
        body: newGateData,
        // The endpoint replies 200 with a plain-text body (not JSON); without
        // this the default JSON parser raises PARSING_ERROR and the mutation
        // rejects even though the gate was created. The body is unused — the
        // table refreshes via invalidatesTags.
        responseHandler: 'text',
      }),
      invalidatesTags: ['Gate'],
    }),
    updateGate: builder.mutation({
      query: (gate) => ({
        url: '/api/update-gate',
        method: 'PUT',
        body: gate,
      }),
      invalidatesTags: ['Gate'],
    }),
    deleteGate: builder.mutation({
      query: (gateId) => ({
        url: `/api/gates/${gateId}`,
        method: 'DELETE',
      }),
      invalidatesTags: ['Gate'],
    }),
    requestGateStatusChange: builder.mutation({
      query: ({ gateId, workerId, requestedStatus }) => ({
        url: `/api/${gateId}/${workerId}/request-status-change/`,
        method: 'POST',
        body: { requestedStatus },
      }),
      invalidatesTags: ['Gate'],
    }),
    updateGatePriority: builder.mutation({
      query: ({ gateId, priority }) => ({
        url: `/api/update-priority/${gateId}`,
        method: 'PUT',
        body: { priority },
      }),
      invalidatesTags: ['Gate'],
    }),
    getDownlinkCounter: builder.query({
        query: () => '/api/downlinkcounter/counter',
    }),
    tryIncrementDownlinkCounter: builder.mutation({
      query: () => ({
        url: '/api/downlinkcounter/try-increment',
        method: 'POST',
      }),
    }),
    resetDownlinkCounter: builder.mutation({
      query: () => ({
        url: '/api/downlinkcounter/reset',
        method: 'POST',
      }),
    }),
    sendDownlink: builder.mutation({
      query: (payload) => ({
        url: '/api/downlink',
        method: 'POST',
        body: payload,
      }),
      invalidatesTags: ['Gate'],
    }),

    // ── Activities ────────────────────────────────────────
    getActivities: builder.query({
        query: () => '/api/gate-activities',
      providesTags: (result) => result ? result.map((activity) => ({ type: 'Activity', id: activity.id })) : [{ type: 'Activity' }],
    }),
    addActivities: builder.mutation({
      query: (newActivities) => ({
        url: '/api/add-activities/',
        method: 'POST',
        body: newActivities,
      }),
      invalidatesTags: ['Activity'],
    }),

    // ── Notifications ─────────────────────────────────────
    getNotifications: builder.query({
        query: () => '/api/notifications',
      providesTags: (result) => result ? result.map((notification) => ({ type: 'Notification', id: notification.id })) : [{ type: 'Notification' }],
    }),
    getNotificationsByWorkerId: builder.query({
        query: (workerId) => `/api/notifications/${workerId}`,
      providesTags: (result) => result ? result.map((notification) => ({ type: 'Notification', id: notification.id })) : [{ type: 'Notification' }],
    }),
    markNotificationAsRead: builder.mutation({
      query: (notificationId) => ({
        url: `/api/notifications/${notificationId}/request-read-change`,
        method: 'POST',
      }),
      invalidatesTags: ['Notification'],
    }),
  }),
});

export const {
  useLoginMutation,
  useRegisterMutation,
  useGetUserDetailsQuery,
  useLazyGetUserDetailsQuery,
  useUpdateUserDetailsMutation,
  useLogoutMutation,
  useGetGatesQuery,
  useCreateGateMutation,
  useUpdateGateMutation,
  useDeleteGateMutation,
  useRequestGateStatusChangeMutation,
  useUpdateGatePriorityMutation,
  useGetDownlinkCounterQuery,
  useTryIncrementDownlinkCounterMutation,
  useResetDownlinkCounterMutation,
  useSendDownlinkMutation,
  useGetActivitiesQuery,
  useAddActivitiesMutation,
  useGetNotificationsQuery,
  useGetNotificationsByWorkerIdQuery,
  useMarkNotificationAsReadMutation,
} = api;