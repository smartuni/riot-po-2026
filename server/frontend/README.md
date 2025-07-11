# Server Team Front-End

This is the readme for the front-end of the Server side team. It will describe the structure of the directory and where to find important files and functions.

## Intro
The front-end of this project serves to provide a reliable, intuitive and straight forward GUI for the use of lay people within the Port management team. It connects to the associated backend and makes API calls and uses WebSockets to communicate to the system to populate it and provide input and interactivity to the user.

## Flow
Users begin their journey at the root page, where they will be presented with the landing page of the Project that has a brief description of the project and includes the Login, Register and Continue as guest buttons.

Once continuing with one of the options (in our case register) and then following the instructions and choosing to become a Controller (which grants extra rights) it will bring you to the Dashboard (/dashboard).

The Dashboard shows users the current state of the system, with an overview of the number of open and closed gates, as well as the primary Status Table which dipslays most of the information and the controls to change the state of the system.

The Status Table is the most important part of the web application and contains a number of fields about each gate, including ID, location, Status, Requested Status, Pending Jobs, Priority, Last Update, Confidence, Actions, Activities, Delete. It allows you to view the status of each gate including the current reported status, the current requested status, the confidence in the current state and the activity log of the gate. 

The frontend allows users to individually or as a batch change the requested status of gates, delete gates, add gates and once the state looks satisfactory send the downlink to the devices themselves. There is also a map view that displays the location and status of the gates on an easy to see visual representation of the city.

Below the Status tables is another table that displays the four most recent changes to the system from any of the gates.

Elsewhere on the main Dashboard there is a notifications button that can be opened showing new notifications and allowing the user to view them. There is also a user page button that lets the user view their current acount details.

The user page lets users view their account details including their role, their name and their email address. It also allows them to change their name and password as well as having the log out button at the bottom.

Going back to the landing page, clicking on the continue as guest button directs the user to the (/dashboard-view) page, which contains a lot of the important information from the Controller dashboard but without the buttons that let the user modify the state of the system. It also doesn't include the notifications or userpage button for obvious reasons as the user is not logged in.


## Directory Structure

The structure of this front-end directory contains a number of sub-folders with distinct purposes.

Within the /frontend directory there is a few important files, namely the package.json for the NPM modules, the (/src) source directory that contains all the working implementation and the (/public) directory which contains some prefilled public resources for React.

The /src directory is where almost all of the files are located. It has a number of important files and subdirectories within it. 

App.js is the root of the webapp and contains all the pages and their routes, while index.js contains App.js itself and instantiates the webapp by rendering it with React DOM.

Other files in /src are prefilled files that serve auxillary and testing purposes, as well as global styles within index.css.

The /assets directory contains some png assets used within the webapp to display the different states of the gates.

The /services directory contains api.js, a file with some prewritten API calls that are exported for use in other components.

The /styles directory contains different css files for a number of different pages / components of the webapp, while the files not listed use in-line styles.

The /components directory is where most of the front-end files are contained. Every component, which includes both pages and subcomponents for those pages is located within this directory.

AlertDialog.js contains an abstracted pop-up dialog for when a user completes their change in user details on the user details page.

AlertDialogIllegal.js is similar to AlertDialog.js but is a pop-up for when a user attempts to access the Controller Dashboard as a non-logged in or invalid user. The button on the pop-up directs users back to the landing page so they can log in, register or continue as guest.

Dashboard.js is the main Controller dashboard that has logic to check whether a user is allowed to access it and whether to trigger the AlertDialogIllegal popup, but otherwise mostly contains other components that make up the page.

DashboardGuest.js is a copy of Dashboard but is for the use of users who continue as guest and do not log in, and has a view only version of the status tables and does not have the notifications and user details page buttons.

DashboardView.js is another copy of Dashboard but unlike DashboardGuest is accessed by logging in as a user without controller rights, and has a view only table but allows notifications and access to the user details page.

HeaderBar.js is the headerbar component that also contains the different buttons on the header like Home, Notifications and User details. It also handles the live notification counter as well as the notifications overlay and pop-up confirmations.

HeaderBarGuest.js is similar to HeaderBar.js butn does not contain any of the same buttons and instead has a return button that directs the user to the landing page so they may choose to log in or create an account.

InfoBoxes.js is the component that displays the overrall status of the system at the top of the dashboard with number of gates, current open gates, current closed gates and current out of service gates. It communicates to the backend via a WebSockets to ensure its up to date with the current state of the system.

LandingPage.js is the entire Landing page to the webapp. It contains some information about the project as well as buttons to login, register or continue as guest.

LoginPage.js contains the login page, which allows a user to login and also provides a link to the register page in case the user is not registered.

LogoutButton.js is an abstracted component that is a button to call the backend logout function and return the user to the landing page of the webapp.

MapView.js provides the gate map system, that allows a user to view on a graphical map of Hamburg where the individual gates are located and what their status is, as well as being clickable elements that have overlays that show more detailed information about them.

NotificationPopup.js is the pop-up component / overlay that is used by the HeaderBar when the notifications button is clicked and displays the loggin in users current notifications.

RecentActivity.js is the activity component used by the dashboard that contains the last 4 activities from all/any gates in the system, and is updated using a WebSocket with the backend to make sure it is constantly accurate.

RegisterPage.js is similar to LoginPage.js but instead contains more input fields and handles the registering functionality. It contains a slider that allows the user to decide what role they should be assigned, either a viewer, or a controller. Like the LoginPage it also has a link to the LoginPage to allow users to login instead of register if they already have an account.

StatusChangedDialog.js is the overlay / pop-up that occurs when you want to change the state of a gate, and allows you to view the current state as well as changing the state and confirming it.

StatusTables.js is the most important file in the front-end as it displays all the important information about the system as well as linking in all the other important components like the MapView, StatusChangeDialog and more. It contains all the buttons to select gates, create new gates, search gates, modify the requested status of gates, send downlinks to the gates and technicians and more. It also contains the activities of each individual gate in a fold out section that also uses WebSockets to update. The overall status of the system also uses WebSockets to update, including whether a gates state has been changed by a Gate technician, or a job has been completed or any other state in the system has changed. It also displays the confidence rating for each gates current state based on previously received reports and calculated in the backend before being sent to the front-end to be displayed.

StatusTablesView.js is similar to StatusTables.js but contains view only information and does not allow the user to modify the system in any way. To that end it removes the buttons seen on the regular StatusTables.js and only contains information on the system-state.

UserPage.js is the page where a user can view their current account details like email, name and role as well as change their password or name. It also contains the LogoutButton.js to allow users to log out.



This project was bootstrapped with [Create React App](https://github.com/facebook/create-react-app).

## Available Scripts

In the project directory (frontend/src), you can run:

### `npm start`

Which runs the app in the development mode.
Open [http://localhost:3000](http://localhost:3000) to view it in your browser.

### `npm run build`

Builds the app for production to the `build` folder.\
It correctly bundles React in production mode and optimizes the build for the best performance.

The build is minified and the filenames include the hashes.\


README completed by Liam Lynch and Andre Demir.