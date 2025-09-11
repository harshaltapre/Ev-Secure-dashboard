import { NextRequest, NextResponse } from 'next/server'

// In-memory storage for API keys
const apiKeys = new Map<string, { stationId: string; lastUsed: Date; status: 'active' | 'inactive'; createdAt: Date }>()

// Initialize with default API keys
apiKeys.set('vsr_st001_abc123def456', { stationId: 'ST001', lastUsed: new Date(), status: 'active', createdAt: new Date() })
apiKeys.set('vsr_st002_xyz789ghi012', { stationId: 'ST002', lastUsed: new Date(), status: 'active', createdAt: new Date() })
apiKeys.set('vsr_st003_mno345pqr678', { stationId: 'ST003', lastUsed: new Date(), status: 'active', createdAt: new Date() })
apiKeys.set('vsr_st004_stu901vwx234', { stationId: 'ST004', lastUsed: new Date(), status: 'active', createdAt: new Date() })

export async function GET(request: NextRequest) {
  try {
    // Get all API keys for the dashboard
    const keysArray = Array.from(apiKeys.entries()).map(([key, data]) => ({
      key: key.substring(0, 10) + '...', // Partial key for security
      fullKey: key,
      stationId: data.stationId,
      status: data.status,
      lastUsed: data.lastUsed,
      createdAt: data.createdAt
    }))

    return NextResponse.json({
      success: true,
      keys: keysArray
    })

  } catch (error) {
    console.error('Error retrieving API keys:', error)
    return NextResponse.json(
      { error: 'Internal server error' },
      { status: 500 }
    )
  }
}

export async function POST(request: NextRequest) {
  try {
    const body = await request.json()
    
    if (!body.stationId) {
      return NextResponse.json(
        { error: 'Missing required field: stationId' },
        { status: 400 }
      )
    }

    // Generate new API key
    const newKey = `vsr_${body.stationId.toLowerCase()}_${Math.random().toString(36).substring(2, 15)}`
    
    // Check if station already has an active key
    const existingKey = Array.from(apiKeys.entries()).find(([_, data]) => 
      data.stationId === body.stationId && data.status === 'active'
    )

    if (existingKey) {
      // Deactivate existing key
      const [oldKey, oldData] = existingKey
      oldData.status = 'inactive'
      apiKeys.set(oldKey, oldData)
    }

    // Add new key
    apiKeys.set(newKey, {
      stationId: body.stationId,
      lastUsed: new Date(),
      status: 'active',
      createdAt: new Date()
    })

    console.log(`New API key generated for ${body.stationId}:`, newKey.substring(0, 10) + '...')

    return NextResponse.json({
      success: true,
      message: 'API key generated successfully',
      key: newKey,
      stationId: body.stationId,
      createdAt: new Date().toISOString()
    })

  } catch (error) {
    console.error('Error generating API key:', error)
    return NextResponse.json(
      { error: 'Internal server error' },
      { status: 500 }
    )
  }
}

export async function DELETE(request: NextRequest) {
  try {
    const body = await request.json()
    
    if (!body.key) {
      return NextResponse.json(
        { error: 'Missing required field: key' },
        { status: 400 }
      )
    }

    const keyData = apiKeys.get(body.key)
    
    if (!keyData) {
      return NextResponse.json(
        { error: 'API key not found' },
        { status: 404 }
      )
    }

    // Revoke the key
    keyData.status = 'inactive'
    apiKeys.set(body.key, keyData)

    console.log(`API key revoked for ${keyData.stationId}:`, body.key.substring(0, 10) + '...')

    return NextResponse.json({
      success: true,
      message: 'API key revoked successfully',
      stationId: keyData.stationId
    })

  } catch (error) {
    console.error('Error revoking API key:', error)
    return NextResponse.json(
      { error: 'Internal server error' },
      { status: 500 }
    )
  }
}
