---
name: better-auth-best-practices
description: Authentication and authorization best practices for secure user management
---

# Better Auth Best Practices

Secure authentication patterns and implementation guidelines.

## When to Use

- Implementing user authentication
- Setting up OAuth/social login
- Configuring session management
- Implementing role-based access control

## Authentication Patterns

### Secure Password Handling

```typescript
// ✅ Use bcrypt or argon2 for password hashing
import { hash, verify } from '@node-rs/argon2';

async function hashPassword(password: string): Promise<string> {
  return hash(password, {
    memoryCost: 65536,
    timeCost: 3,
    parallelism: 4
  });
}

async function verifyPassword(hash: string, password: string): Promise<boolean> {
  return verify(hash, password);
}

// ✅ Password requirements
const passwordSchema = z.string()
  .min(8, 'Password must be at least 8 characters')
  .regex(/[A-Z]/, 'Must contain uppercase letter')
  .regex(/[a-z]/, 'Must contain lowercase letter')
  .regex(/[0-9]/, 'Must contain number');
```

### Session Management

```typescript
// ✅ Secure session configuration
const sessionConfig = {
  secret: process.env.SESSION_SECRET!, // 32+ random bytes
  name: '__session', // Custom cookie name
  cookie: {
    httpOnly: true,
    secure: process.env.NODE_ENV === 'production',
    sameSite: 'lax' as const,
    maxAge: 7 * 24 * 60 * 60 * 1000, // 7 days
    path: '/'
  },
  rolling: true // Extend session on activity
};

// ✅ Session invalidation on sensitive actions
async function changePassword(userId: string, newPassword: string) {
  await updatePassword(userId, newPassword);
  await invalidateAllSessions(userId); // Force re-login
}

// ✅ Concurrent session limits
async function createSession(userId: string) {
  const activeSessions = await getActiveSessions(userId);
  if (activeSessions.length >= 5) {
    await invalidateOldestSession(userId);
  }
  return createNewSession(userId);
}
```

### OAuth/Social Login

```typescript
// ✅ State parameter for CSRF protection
async function initiateOAuth(provider: string) {
  const state = crypto.randomBytes(32).toString('hex');
  await redis.set(`oauth:state:${state}`, 'pending', 'EX', 600);
  
  return redirectToProvider(provider, { state });
}

// ✅ Verify state on callback
async function handleOAuthCallback(code: string, state: string) {
  const storedState = await redis.get(`oauth:state:${state}`);
  if (!storedState) {
    throw new Error('Invalid or expired state');
  }
  await redis.del(`oauth:state:${state}`);
  
  return exchangeCodeForTokens(code);
}

// ✅ Link accounts securely
async function linkSocialAccount(userId: string, provider: string, providerUserId: string) {
  // Check if already linked to another user
  const existing = await findByProvider(provider, providerUserId);
  if (existing && existing.userId !== userId) {
    throw new Error('Account already linked to another user');
  }
  
  await createLink(userId, provider, providerUserId);
}
```

### JWT Best Practices

```typescript
// ✅ Short-lived access tokens + refresh tokens
const ACCESS_TOKEN_EXPIRY = '15m';
const REFRESH_TOKEN_EXPIRY = '7d';

function generateTokens(userId: string) {
  const accessToken = jwt.sign(
    { sub: userId, type: 'access' },
    process.env.JWT_SECRET!,
    { expiresIn: ACCESS_TOKEN_EXPIRY }
  );
  
  const refreshToken = jwt.sign(
    { sub: userId, type: 'refresh', jti: crypto.randomUUID() },
    process.env.JWT_REFRESH_SECRET!,
    { expiresIn: REFRESH_TOKEN_EXPIRY }
  );
  
  return { accessToken, refreshToken };
}

// ✅ Refresh token rotation
async function refreshAccessToken(refreshToken: string) {
  const payload = jwt.verify(refreshToken, process.env.JWT_REFRESH_SECRET!);
  
  // Check if token is revoked
  const isRevoked = await isTokenRevoked(payload.jti);
  if (isRevoked) {
    throw new Error('Token revoked');
  }
  
  // Revoke old refresh token
  await revokeToken(payload.jti);
  
  // Issue new tokens
  return generateTokens(payload.sub);
}
```

### Multi-Factor Authentication

```typescript
// ✅ TOTP implementation
import { authenticator } from 'otplib';

function generateTOTPSecret(): string {
  return authenticator.generateSecret();
}

function verifyTOTP(secret: string, token: string): boolean {
  return authenticator.verify({ token, secret });
}

// ✅ Backup codes
function generateBackupCodes(): string[] {
  return Array.from({ length: 10 }, () => 
    crypto.randomBytes(4).toString('hex').toUpperCase()
  );
}

// ✅ Rate limit MFA attempts
async function verifyMFA(userId: string, code: string) {
  const attempts = await getAttempts(userId);
  if (attempts >= 5) {
    throw new Error('Too many attempts. Try again later.');
  }
  
  const user = await getUser(userId);
  const valid = verifyTOTP(user.totpSecret, code);
  
  if (!valid) {
    await incrementAttempts(userId);
    throw new Error('Invalid code');
  }
  
  await resetAttempts(userId);
  return true;
}
```

## Authorization Patterns

### Role-Based Access Control (RBAC)

```typescript
// ✅ Define roles and permissions
const PERMISSIONS = {
  'posts:read': ['user', 'editor', 'admin'],
  'posts:write': ['editor', 'admin'],
  'posts:delete': ['admin'],
  'users:manage': ['admin']
} as const;

function hasPermission(userRole: string, permission: string): boolean {
  return PERMISSIONS[permission]?.includes(userRole) ?? false;
}

// ✅ Middleware for route protection
function requirePermission(permission: string) {
  return (req: Request, res: Response, next: NextFunction) => {
    if (!hasPermission(req.user.role, permission)) {
      return res.status(403).json({ error: 'Forbidden' });
    }
    next();
  };
}

// Usage
app.delete('/posts/:id', requirePermission('posts:delete'), deletePost);
```

### Resource-Based Authorization

```typescript
// ✅ Check ownership before action
async function updatePost(userId: string, postId: string, data: PostData) {
  const post = await getPost(postId);
  
  if (!post) {
    throw new NotFoundError('Post not found');
  }
  
  if (post.authorId !== userId && !isAdmin(userId)) {
    throw new ForbiddenError('Not authorized to edit this post');
  }
  
  return db.posts.update({ where: { id: postId }, data });
}
```

## Security Checklist

- [ ] Passwords hashed with bcrypt/argon2 (cost factor 10+)
- [ ] Sessions use httpOnly, secure, sameSite cookies
- [ ] CSRF protection on state-changing endpoints
- [ ] Rate limiting on login/registration endpoints
- [ ] Account lockout after failed attempts
- [ ] Secure password reset flow (time-limited tokens)
- [ ] OAuth state parameter validated
- [ ] JWT refresh tokens rotated on use
- [ ] MFA available for sensitive accounts
- [ ] Audit logging for auth events
