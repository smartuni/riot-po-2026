import React, { useState } from 'react';
import { FiHome, FiLock, FiMap, FiClipboard, FiActivity, FiUser } from 'react-icons/fi';
import '../styles/Sidebar.css';
import { useNavigate, useLocation } from 'react-router-dom';

const menuItems = [
    { id: 1, name: "Overview", icon: <FiHome />, path: "/dashboard" },
    { id: 2, name: "Gates", icon: <FiLock />, path: "/gates" },
    { id: 3, name: "Map View", icon: <FiMap />, path: "/map-view" },
    { id: 4, name: "Event Logs", icon: <FiClipboard />, path: "/event-logs" },
];

function Sidebar() {
    const location = useLocation()
    console.log(location.pathname);
    const matchedId = menuItems.find((a) => a.path === location.pathname).id;
    const [active, setActive] = useState(matchedId);
    const navigate = useNavigate();
    function handleClick(item) {
        setActive(item.id);
        navigate(`${item.path}`)
    }
    return (
        <div className="sidebar">
            <nav className="sidebar-menu">
                {menuItems.map((item) => (
                    <div
                        key={item.id}
                        className={`sidebar-item ${active === item.id ? 'active' : ''}`}
                        onClick={() => handleClick(item)}
                    >
                        <div className="sidebar-icon">{item.icon}</div>
                        <span className="sidebar-text">{item.name}</span>
                    </div>
                ))}
            </nav>
        </div>
    );
}

export default Sidebar;
