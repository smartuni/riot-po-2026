import { createSlice } from '@reduxjs/toolkit';

const initialState = {
  activities: [],
};

const activitiesSlice = createSlice({
  name: 'activities',
  initialState,
  reducers: {
    activityAdded(state, action) {
      state.activities.push(action.payload);
    },
    activityDeleted(state, action) {
      const id = parseInt(action.payload);
      state.activities = state.activities.filter(
        (activity) => activity.id !== id
      );
    },
  },
});

export const { activityAdded, activityDeleted } = activitiesSlice.actions;
export default activitiesSlice.reducer;
