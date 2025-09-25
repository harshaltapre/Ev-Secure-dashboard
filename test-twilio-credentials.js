// Test Twilio credentials
require('dotenv').config({ path: '.env.local' })

console.log('Testing Twilio Credentials...\n')

console.log('TWILIO_ACCOUNT_SID:', process.env.TWILIO_ACCOUNT_SID)
console.log('TWILIO_AUTH_TOKEN:', process.env.TWILIO_AUTH_TOKEN ? '***' + process.env.TWILIO_AUTH_TOKEN.slice(-4) : 'Not set')
console.log('TWILIO_PHONE_NUMBER:', process.env.TWILIO_PHONE_NUMBER)

console.log('\nValidation:')
console.log('✅ Account SID starts with AC:', process.env.TWILIO_ACCOUNT_SID?.startsWith('AC'))
console.log('✅ Auth Token exists:', !!process.env.TWILIO_AUTH_TOKEN)
console.log('✅ Phone number format:', /^\+91\d{10}$/.test(process.env.TWILIO_PHONE_NUMBER || ''))

if (process.env.TWILIO_ACCOUNT_SID?.startsWith('AC') && process.env.TWILIO_AUTH_TOKEN) {
  console.log('\n🎉 Credentials look valid!')
} else {
  console.log('\n❌ Credentials need to be updated in .env.local')
}
