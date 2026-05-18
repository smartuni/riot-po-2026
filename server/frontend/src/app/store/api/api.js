import { createApi, fetchBaseQuery } from '@reduxjs/toolkit/query/react';
import { setCookie, eraseCookie } from '../../../shared/utils/cookie';

export const api = createApi({
  reducerPath: 'api',
  baseQuery: fetchBaseQuery({
    baseUrl: import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080',
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
        url: '/auth/login',
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
        url: '/auth/register',
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
      query: () => '/auth/user-details',
      providesTags: ['Auth'],
    }),
    updateUserDetails: builder.mutation({
      query: (body) => ({
        url: '/auth/user-change',
        method: 'PUT',
        body,
      }),
      invalidatesTags: ['Auth'],
    }),
    logout: builder.mutation({
      query: () => ({
        url: '/auth/logout',
        method: 'POST',
      }),
      onQueryStarted: async (_, { queryFulfilled, dispatch }) => {
        await queryFulfilled;
        dispatch({ type: 'auth/clearToken' });
        eraseCookie('jwt');
      },
    }),

    // ── Gates ─────────────────────────────────────────────
    getGates: builder.query({
      query: () => '/gates',
      providesTags: (result) => result ? result.map((gate) => ({ type: 'Gate', id: gate.id })) : [{ type: 'Gate' }],
    }),
    createGate: builder.mutation({
      query: (newGateData) => ({
        url: '/add-gate-ui',
        method: 'POST',
        body: newGateData,
      }),
      invalidatesTags: ['Gate'],
    }),
    updateGate: builder.mutation({
      query: (gate) => ({
        url: '/update-gate',
        method: 'PUT',
        body: gate,
      }),
      invalidatesTags: ['Gate'],
    }),
    deleteGate: builder.mutation({
      query: (gateId) => ({
        url: `/gates/${gateId}`,
        method: 'DELETE',
      }),
      invalidatesTags: ['Gate'],
    }),
    requestGateStatusChange: builder.mutation({
      query: ({ gateId, workerId, requestedStatus }) => ({
        url: `/${gateId}/${workerId}/request-status-change/`,
        method: 'POST',
        body: { requestedStatus },
      }),
      invalidatesTags: ['Gate'],
    }),
    updateGatePriority: builder.mutation({
      query: ({ gateId, priority }) => ({
        url: `/update-priority/${gateId}`,
        method: 'PUT',
        body: { priority },
      }),
      invalidatesTags: ['Gate'],
    }),
    getDownlinkCounter: builder.query({
      query: () => '/downlinkcounter/counter',
    }),
    tryIncrementDownlinkCounter: builder.mutation({
      query: () => ({
        url: '/downlinkcounter/try-increment',
        method: 'POST',
      }),
    }),
    resetDownlinkCounter: builder.mutation({
      query: () => ({
        url: '/downlinkcounter/reset',
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
      query: () => '/gate-activities',
      providesTags: (result) => result ? result.map((activity) => ({ type: 'Activity', id: activity.id })) : [{ type: 'Activity' }],
    }),
    addActivities: builder.mutation({
      query: (newActivities) => ({
        url: '/add-activities/',
        method: 'POST',
        body: newActivities,
      }),
      invalidatesTags: ['Activity'],
    }),

    // ── Notifications ─────────────────────────────────────
    getNotifications: builder.query({
      query: () => '/notifications',
      providesTags: (result) => result ? result.map((notification) => ({ type: 'Notification', id: notification.id })) : [{ type: 'Notification' }],
    }),
    getNotificationsByWorkerId: builder.query({
      query: (workerId) => `/notifications/${workerId}`,
      providesTags: (result) => result ? result.map((notification) => ({ type: 'Notification', id: notification.id })) : [{ type: 'Notification' }],
    }),
    markNotificationAsRead: builder.mutation({
      query: (notificationId) => ({
        url: `/notifications/${notificationId}/request-read-change`,
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