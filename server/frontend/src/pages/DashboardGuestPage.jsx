import React from 'react';
import { Link } from 'react-router-dom';
import StatusTablesView from '../features/gates/components/StatusTablesView';

function DashboardGuestPage() {
    return (
        <div className="guest-dashboard">
            <header className="auth-header">
                <Link to="/">
                    <div className="logo-icon">SM</div>
                    <span>SenseMate</span>
                </Link>
                <Link to="/login" className="btn btn-primary">Sign In</Link>
            </header>
            <main className="guest-content" style={{ padding: '24px', maxWidth: '1200px', margin: '0 auto' }}>
                <StatusTablesView />
            </main>
        </div>
    );
}

export default DashboardGuestPage;
