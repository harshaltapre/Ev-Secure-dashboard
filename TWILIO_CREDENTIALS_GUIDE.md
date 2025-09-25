# Twilio Credentials Setup Guide

## 📍 Where to Put Twilio API Credentials

### File: `.env.local` (in your project root)

## 🔧 How to Update Credentials

### Step 1: Open `.env.local` file
- Navigate to your project folder: `C:\Users\Victus\Desktop\Harshal\Ev-Secure-dashboard`
- Open the file `.env.local` in any text editor

### Step 2: Update the values
Replace the placeholder values with your actual Twilio credentials:

```env
# JWT Secret for authentication
JWT_SECRET=your-secret-key-change-in-production

# Twilio Configuration for SMS
TWILIO_ACCOUNT_SID=ACef406b1846f5f7435c97a531d5196cf1
TWILIO_AUTH_TOKEN=b4850b58caedcefe567e523c30e5ae88
TWILIO_PHONE_NUMBER=+919322184006
```

## ⚠️ Important Notes

### Phone Number Format
- ❌ Wrong: `+91 9322184006` (with space)
- ✅ Correct: `+919322184006` (without space)

### Credential Sources
- **Account SID**: From Twilio Console Dashboard
- **Auth Token**: From Twilio Console Dashboard (click to reveal)
- **Phone Number**: Must be purchased from Twilio

## 🧪 Testing

### After updating credentials:
1. **Restart server**: `npm run dev`
2. **Test SMS**: Go to login page → "Forgot Password"
3. **Enter phone**: `+919322184006`
4. **Check phone**: You should receive SMS with OTP

### Test Script:
```bash
node test-sms.js
```

## 🔍 Troubleshooting

### If SMS not received:
1. Check Twilio Console for delivery status
2. Verify phone number format (no spaces)
3. Ensure Twilio account has credits
4. Check if phone number is verified in Twilio

### Common Issues:
- **Invalid credentials**: Double-check Account SID and Auth Token
- **Phone format**: Remove spaces from phone number
- **No credits**: Add funds to Twilio account
- **Unverified number**: Verify destination phone in Twilio Console

## 📱 Current Status

✅ **SMS Infrastructure**: Ready
✅ **OTP Generation**: Working
✅ **Phone Validation**: Working
⏳ **Real SMS**: Waiting for Twilio credentials update

---

**Next Step**: Update `.env.local` with correct Twilio credentials and restart server.
