import { Link } from 'react-router-dom';

export default function NotFoundPage() {
    return (
        <div className="card" style={{ textAlign: 'center', padding: '60px 24px', maxWidth: 480, margin: '0 auto' }}>
            <div style={{ fontSize: 48, marginBottom: 16 }}>🔍</div>
            <h1 style={{ fontSize: 24, fontWeight: 700, margin: '0 0 8px' }}>Page Not Found</h1>
            <p style={{ color: 'var(--text-secondary)', marginBottom: 24 }}>
                The page you're looking for doesn't exist.
            </p>
            <Link to="/" className="btn btn-primary" style={{ display: 'inline-block' }}>
                Back to Home
            </Link>
        </div>
    );
}
